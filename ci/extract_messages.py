#!/usr/bin/env python3
"""Extract literal GLogger templates into YAML catalogs, or validate a translated catalog.

Requires PyYAML (available in GEMC's build Python environment). This scans source text rather than
preprocessed C++, so messages in all conditional branches are collected. It does not invoke agents.
"""

import argparse
from dataclasses import dataclass
import fnmatch
import json
from pathlib import Path
import re
import sys

import yaml


METHODS = {"info", "debug", "warning", "error", "critical", "format"}
RECEIVERS = {"log", "logger", "glog", "glogger", "caller_log"}
SUFFIXES = {".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
EXCLUDED_DIRS = {".git", "build", "subprojects"}
TOKEN = re.compile(
    r'(?P<comment>//[^\n]*|/\*.*?\*/)'
    r'|(?P<raw>(?:u8|u|U|L)?R"(?P<delimiter>[^()\s\\]{0,16})\(.*?\)(?P=delimiter)")'
    r'|(?P<string>(?:u8|u|U|L)?"(?:\\.|[^"\\])*")'
    r"|(?P<char>(?:u8|u|U|L)?'(?:\\.|[^'\\])*')"
    r'|(?P<identifier>[A-Za-z_]\w*)'
    r'|(?P<operator>->|::|[^\s])',
    re.S,
)


@dataclass(frozen=True)
class Token:
    text: str
    kind: str
    offset: int


def tokenize(source):
    return [Token(m.group(), m.lastgroup, m.start()) for m in TOKEN.finditer(source)
            if m.lastgroup != "comment"]


def decode_string(token):
    """Decode narrow C++ literals, preserving UTF-8 bytes and C++ escape semantics."""
    if token.kind == "raw":
        if not token.text.startswith('R"'):
            raise ValueError("only narrow string literals are supported")
        opening = token.text.index("(")
        delimiter = token.text[2:opening]
        return token.text[opening + 1:-(len(delimiter) + 2)]
    if not token.text.startswith('"'):
        raise ValueError("only narrow string literals are supported")
    body = token.text[1:-1]
    escapes = {"a": 7, "b": 8, "f": 12, "n": 10, "r": 13, "t": 9, "v": 11,
               "\\": 92, '"': 34, "'": 39, "?": 63}
    result = bytearray()
    index = 0
    while index < len(body):
        if body[index] != "\\":
            result.extend(body[index].encode("utf-8"))
            index += 1
            continue
        index += 1
        if index == len(body):
            raise ValueError("unterminated string escape")
        char = body[index]
        index += 1
        if char in escapes:
            result.append(escapes[char])
        elif char == "\n":
            continue  # C++ line continuation.
        elif char == "\r" and body[index:index + 1] == "\n":
            index += 1
        elif char in "01234567":
            end = index
            while end < min(index + 2, len(body)) and body[end] in "01234567":
                end += 1
            result.append(int(char + body[index:end], 8))
            index = end
        elif char == "x":
            match = re.match(r"[0-9a-fA-F]+", body[index:])
            if not match:
                raise ValueError("hexadecimal escape has no digits")
            result.append(int(match.group(), 16))
            index += len(match.group())
        elif char in "uU":
            length = 4 if char == "u" else 8
            digits = body[index:index + length]
            if len(digits) != length or not re.fullmatch(r"[0-9a-fA-F]+", digits):
                raise ValueError("invalid Unicode escape")
            result.extend(chr(int(digits, 16)).encode("utf-8"))
            index += length
        else:
            raise ValueError(f"unsupported C++ escape: \\{char}")
    return result.decode("utf-8")


def literal(tokens):
    if not tokens or any(t.kind not in {"string", "raw"} for t in tokens):
        return None
    return "".join(decode_string(t) for t in tokens)


def call_arguments(tokens, opening):
    """Split arguments without splitting nested calls, initializers, or lambdas."""
    pairs = {"(": ")", "[": "]", "{": "}"}
    stack = []
    arguments = []
    start = opening + 1
    for index in range(start, len(tokens)):
        text = tokens[index].text
        if tokens[index].kind in {"string", "raw", "char"}:
            continue
        if text in pairs:
            stack.append(pairs[text])
        elif text in pairs.values():
            if not stack:
                if text != ")":
                    raise ValueError("unbalanced logger call")
                if index > start:
                    arguments.append(tokens[start:index])
                return arguments
            if stack.pop() != text:
                raise ValueError("unbalanced logger call")
        elif text == "," and not stack:
            arguments.append(tokens[start:index])
            start = index + 1
    raise ValueError("unterminated logger call")


def placeholders(message):
    """Match GLogger's %1..%99 and %% rules; also return the non-parameter text."""
    used = set()
    text = []
    index = 0
    while index < len(message):
        if message[index] != "%" or index + 1 == len(message):
            text.append(message[index])
            index += 1
        elif message[index + 1] == "%":
            text.append("%")
            index += 2
        elif message[index + 1] not in "0123456789":
            text.append("%")
            index += 1
        else:
            match = re.match(r"[0-9]+", message[index + 1:])
            digits = match.group()
            if digits.startswith("0") or len(digits) > 2:
                raise ValueError(f"invalid placeholder %{digits}")
            used.add(int(digits))
            index += 1 + len(digits)
    return used, "".join(text)


def extract_file(path, root, receivers):
    source = path.read_text(encoding="utf-8")
    tokens = tokenize(source)
    messages = {}
    for index, token in enumerate(tokens[:-1]):
        method = token.text.removesuffix("Template")
        if method not in METHODS or tokens[index + 1].text != "(":
            continue
        if method == "format" and path.name != "goptions.cc":
            continue  # The logger's formatter wrapper forwards an already extracted template.
        member = index >= 2 and tokens[index - 1].text in {".", "->"}
        if member:
            receiver = tokens[index - 2].text
            if receiver not in receivers and not (method == "format" and receiver == "catalog"):
                continue
        elif not (path.name in {"glogger.h", "glogger.cc"} and method == "debug"
                  and tokens[index + 2].text in {"CONSTRUCTOR", "DESTRUCTOR"}):
            continue
        location = f"{path.relative_to(root).as_posix()}:{source.count(chr(10), 0, token.offset) + 1}"
        try:
            args = call_arguments(tokens, index + 1)
            prefix = 1 if method in {"debug", "error"} else 0
            if method == "info" and args and literal(args[0]) is None:
                prefix = 1  # The explicit-level overload, including a nonliteral level expression.
            if len(args) <= prefix:
                raise ValueError("logger call has no template")
            message = literal(args[prefix])
            if message is None:
                raise ValueError('template must be literal; use "%1" for dynamic diagnostic text')
            used, prose = placeholders(message)
            count = len(args) - prefix - 1
            if count > 99 or used != set(range(1, count + 1)):
                raise ValueError("placeholders do not match the supplied parameters")
            if any(char.isalpha() for char in prose):
                messages.setdefault(message, []).append(location)
        except (ValueError, UnicodeError) as error:
            raise ValueError(f"{location}: {error}") from error
    return messages


class UniqueLoader(yaml.SafeLoader):
    """Reject duplicate keys instead of silently losing catalog messages."""


def unique_mapping(loader, node):
    result = {}
    for key_node, value_node in node.value:
        key = loader.construct_object(key_node)
        if key in result:
            raise ValueError(f"duplicate YAML key: {key!r}")
        result[key] = loader.construct_object(value_node)
    return result


UniqueLoader.add_constructor(yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG, unique_mapping)


def load_yaml(path):
    return yaml.load(path.read_text(encoding="utf-8"), Loader=UniqueLoader)


def quoted_lines(value, prefix):
    """Wrap a YAML double-quoted scalar without altering spaces, escapes, or long URLs."""
    encoded = json.dumps(value, ensure_ascii=False)[1:-1]
    atoms = re.findall(r'\\(?:u[0-9a-fA-F]{4}|.)|[^\\]', encoded)
    lines = []
    line = prefix + '"'
    for atom in atoms:
        if len(line) + len(atom) > 108:
            lines.append(line + "\\")
            line = "    "
            if atom == " ":
                atom = "\\ "  # Preserve leading whitespace after the escaped line break.
        line += atom
    return lines + [line + '"']


def render_catalog(messages):
    lines = ["# Generated by ci/extract_messages.py; edit English templates in C++ instead.",
             "# Source locations are comments; message keys and values preserve the exact template text.",
             "schema: 1", "language: en", "messages:"]
    if not messages:
        lines[-1] = "messages: {}"
    for message, locations in sorted(messages.items()):
        lines.extend("  # " + location for location in sorted(set(locations)))
        quoted = json.dumps(message, ensure_ascii=False)
        if len(quoted) * 2 + 4 <= 112:
            lines.append(f"  {quoted}: {quoted}")
        else:
            lines.extend(quoted_lines(message, "  ? "))
            lines.extend(quoted_lines(message, "  : "))
    return "\n".join(lines) + "\n"


def repository_path(root, name):
    path = (root / name).resolve()
    relative = path.relative_to(root)
    if EXCLUDED_DIRS.intersection(relative.parts):
        raise ValueError(f"path must belong to the owning source tree: {name}")
    return path


def extract(root, check):
    root = root.resolve()
    config = load_yaml(root / "translations/catalog.yaml")
    if not isinstance(config, dict) or config.get("schema") != 1:
        raise ValueError("source manifest must use schema 1")
    sources = config.get("sources")
    if not isinstance(sources, list) or not sources or not all(isinstance(p, str) for p in sources):
        raise ValueError("source manifest needs a nonempty sources list")
    excluded = config.get("exclude", [])
    receivers = set(config.get("receivers", sorted(RECEIVERS)))
    paths = set()
    for name in sources:
        source = repository_path(root, name)
        if not source.exists():
            raise ValueError(f"source path does not exist: {name}")
        candidates = source.rglob("*") if source.is_dir() else [source]
        for path in candidates:
            relative = path.relative_to(root)
            if path.suffix not in SUFFIXES or EXCLUDED_DIRS.intersection(relative.parts):
                continue
            if any(fnmatch.fnmatchcase(relative.as_posix(), pattern) for pattern in excluded):
                continue
            resolved = repository_path(root, relative)
            if resolved.is_file():
                paths.add(resolved)
    messages = {}
    for path in sorted(paths):
        for message, locations in extract_file(path, root, receivers).items():
            messages.setdefault(message, []).extend(locations)
    catalog = render_catalog(messages)
    parsed = yaml.load(catalog, Loader=UniqueLoader)
    if parsed["messages"] != {message: message for message in messages}:
        raise ValueError("catalog serialization changed the runtime template text")
    output = repository_path(root, "translations/en.yaml")
    if check:
        if not output.is_file() or output.read_text(encoding="utf-8") != catalog:
            raise ValueError(f"{output}: catalog is stale; run extract without --check")
    else:
        output.write_text(catalog, encoding="utf-8")
    print(f"{'Checked' if check else 'Generated'} {output}: {len(messages)} messages from {len(paths)} files.")


def validate(path, source=None):
    catalog = load_yaml(path)
    if not isinstance(catalog, dict) or type(catalog.get("schema")) is not int or catalog["schema"] != 1:
        raise ValueError("catalog must use schema 1")
    if not isinstance(catalog.get("language"), str) or not catalog["language"]:
        raise ValueError("catalog needs a language code")
    messages = catalog.get("messages")
    if not isinstance(messages, dict):
        raise ValueError("catalog messages must be a mapping")
    known = None
    if source:
        known = validate(source)
    for message, translation in messages.items():
        if not isinstance(message, str) or not isinstance(translation, str) or not translation.strip():
            raise ValueError("message keys and translated values must be nonempty strings")
        if not message.strip():
            raise ValueError("message keys must be nonempty")
        if known is not None and message not in known:
            raise ValueError(f"unknown English template: {message!r}")
        if placeholders(message)[0] != placeholders(translation)[0]:
            raise ValueError(f"translation changes the placeholders: {message!r}")
        if message.count("%%") != translation.count("%%"):
            raise ValueError(f"translation changes literal percent escapes: {message!r}")
        for method in ("lstrip", "rstrip"):
            original = getattr(message, method)(" \t\r\n")
            translated = getattr(translation, method)(" \t\r\n")
            if method == "lstrip":
                original = message[:len(message) - len(original)]
                translated = translation[:len(translation) - len(translated)]
            else:
                original = message[len(original):]
                translated = translation[len(translated):]
            if original != translated:
                raise ValueError(f"translation changes surrounding whitespace: {message!r}")
        if any(message.count(char) != translation.count(char) for char in ("\n", "\r", "\t")):
            raise ValueError(f"translation changes line breaks or tabs: {message!r}")
        if catalog["language"] == "en" and message != translation:
            raise ValueError(f"English catalog value differs from its key: {message!r}")
    print(f"Validated {path}: {len(messages)} messages ({catalog['language']}).")
    return messages


def validate_tree(root):
    """Check freshness and every language catalog without writing files or calling agents."""
    root = root.resolve()
    extract(root, check=True)
    directory = root / "translations"
    source = directory / "en.yaml"
    english = validate(source)
    config = load_yaml(directory / "catalog.yaml")
    required = config.get("languages", [])
    if not isinstance(required, list) or not all(isinstance(language, str) for language in required):
        raise ValueError("manifest languages must be a list of language codes")
    paths = sorted(path for path in directory.glob("*.yaml") if path.name != "catalog.yaml")
    available = {path.stem for path in paths}
    missing = set(required) - available
    if missing:
        raise ValueError("missing language catalogs: " + ", ".join(sorted(missing)))
    for path in paths:
        data = load_yaml(path)
        if not re.fullmatch(r"[a-z]{2,3}(?:-[a-z0-9]+)*", path.stem):
            raise ValueError(f"{path}: filename must use a normalized language code")
        if not isinstance(data, dict) or data.get("language") != path.stem:
            raise ValueError(f"{path}: language metadata must match the filename")
        if path == source:
            continue
        messages = validate(path, source)
        print(f"Coverage {path.stem}: {len(messages)}/{len(english)}; "
              f"{len(english) - len(messages)} messages use English fallback.")
    print(f"Validated {len(paths) - 1} translated catalogs in {directory}.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    extraction = commands.add_parser("extract", help="generate an owning repository's English catalog")
    extraction.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent)
    extraction.add_argument("--check", action="store_true", help="fail if the generated catalog is stale")
    validation = commands.add_parser("validate", help="check YAML structure and placeholder preservation")
    validation.add_argument("catalog", type=Path)
    validation.add_argument("--source", type=Path, help="reject keys absent from the current English catalog")
    tree = commands.add_parser("validate-tree", help="check freshness and all owning language catalogs")
    tree.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    try:
        if args.command == "extract":
            extract(args.root.resolve(), args.check)
        elif args.command == "validate-tree":
            validate_tree(args.root)
        else:
            validate(args.catalog, args.source)
    except (ValueError, OSError, yaml.YAMLError) as error:
        parser.exit(1, f"error: {error}\n")


if __name__ == "__main__":
    main()

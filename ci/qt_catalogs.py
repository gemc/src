#!/usr/bin/env python3
"""Extract and validate owning Qt TS catalogs. Extraction requires lupdate; validation needs only PyYAML."""

import argparse
from collections import Counter
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import xml.etree.ElementTree as ET

import yaml


LANGUAGE_FORMS = {"en": 2, "it": 2, "fr": 2, "ja": 1, "zh": 1}
PARAMETERS = re.compile(r"%(?:L?[0-9]+|L?n)")
MARKUP = re.compile(r"<[^>]+>|&(?:[A-Za-z]+|#x?[0-9a-fA-F]+);")


def message_id(key):
    return json.dumps(key, ensure_ascii=False)


def plural_forms(language):
    base = language.replace("_", "-").split("-")[0]
    if base not in LANGUAGE_FORMS:
        raise ValueError(f"plural rules are not configured for {language}")
    return LANGUAGE_FORMS[base]


def load(path):
    root = ET.parse(path).getroot()
    if root.tag != "TS" or root.get("version") != "2.1" or not root.get("language"):
        raise ValueError(f"{path}: expected Qt TS 2.1 with language metadata")
    messages = {}
    language = root.get("language")
    for context in root.findall("context"):
        name = context.findtext("name")
        if not name:
            raise ValueError("Qt context needs a name")
        for message in context.findall("message"):
            source = message.findtext("source")
            if not source or not source.strip() or message.get("id"):
                raise ValueError("Qt messages need nonempty source text; ID-based messages are unsupported")
            numerus = message.get("numerus") == "yes"
            key = (name, source, message.findtext("comment", ""), numerus)
            if key in messages:
                raise ValueError(f"duplicate Qt message: {message_id(key)}")
            translation = message.find("translation")
            if translation is None:
                raise ValueError(f"missing translation element: {message_id(key)}")
            values = tuple((item.text or "") for item in translation.findall("numerusform")) if numerus else (
                translation.text or "",)
            unfinished = translation.get("type") == "unfinished"
            if translation.get("type") not in (None, "unfinished"):
                raise ValueError("obsolete or vanished Qt translations must be pruned")
            if not values or not any(values):
                if not unfinished:
                    raise ValueError(f"empty completed Qt translation: {message_id(key)}")
                messages[key] = None
                continue
            if len(values) != (plural_forms(language) if numerus else 1):
                raise ValueError(f"incorrect number of plural forms: {message_id(key)}")
            for value in values:
                validate_message(source, value)
            messages[key] = None if unfinished else values
    return language, messages


def validate_message(source, value):
    if not isinstance(value, str) or not value.strip():
        raise ValueError("Qt translations must be nonempty strings")
    if set(PARAMETERS.findall(source)) != set(PARAMETERS.findall(value)):
        raise ValueError(f"changed Qt placeholders: {source!r}")
    for text in (source, value):
        for parameter in PARAMETERS.findall(text):
            number = parameter.removeprefix("%").removeprefix("L")
            if number != "n" and (number.startswith("0") or int(number) > 99):
                raise ValueError(f"invalid Qt placeholder: {parameter}")
    if source.count("%%") != value.count("%%"):
        raise ValueError(f"changed literal percent signs: {source!r}")
    for method in ("lstrip", "rstrip"):
        def whitespace(text):
            stripped = getattr(text, method)(" \t\r\n")
            return text[:len(text) - len(stripped)] if method == "lstrip" else text[len(stripped):]
        if whitespace(source) != whitespace(value):
            raise ValueError(f"changed surrounding whitespace: {source!r}")
    if any(source.count(char) != value.count(char) for char in ("\n", "\r", "\t")):
        raise ValueError(f"changed line breaks or tabs: {source!r}")
    if Counter(MARKUP.findall(source)) != Counter(MARKUP.findall(value)):
        raise ValueError(f"changed rich-text markup: {source!r}")
    def mnemonics(text):
        return MARKUP.sub("", text).replace("&&", "").count("&")
    if mnemonics(source) != mnemonics(value):
        raise ValueError(f"changed keyboard mnemonic count: {source!r}")
    filters = r"\*\.[A-Za-z0-9]+|\(\*\)"
    if Counter(re.findall(filters, source)) != Counter(re.findall(filters, value)):
        raise ValueError(f"changed file-filter patterns: {source!r}")
    if source.count(";;") != value.count(";;"):
        raise ValueError(f"changed file-filter separators: {source!r}")


def catalog_text(language, keys, translations):
    root = ET.Element("TS", version="2.1", language=language, sourcelanguage="en")
    contexts = {}
    for key in sorted(keys):
        name, source, comment, numerus = key
        if name not in contexts:
            contexts[name] = ET.SubElement(root, "context")
            ET.SubElement(contexts[name], "name").text = name
        message = ET.SubElement(contexts[name], "message", {"numerus": "yes"} if numerus else {})
        ET.SubElement(message, "source").text = source
        if comment:
            ET.SubElement(message, "comment").text = comment
        values = translations.get(key)
        translation = ET.SubElement(message, "translation",
                                    {} if values else {"type": "unfinished"})
        if numerus:
            for index in range(plural_forms(language)):
                ET.SubElement(translation, "numerusform").text = values[index] if values else ""
        else:
            translation.text = values[0] if values else ""
    ET.indent(root, space="  ")
    return ('<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n' +
            ET.tostring(root, encoding="unicode") + "\n")


def manifest(root):
    data = yaml.safe_load((root / "translations/gui/catalog.yaml").read_text(encoding="utf-8"))
    if not isinstance(data, dict) or data.get("schema") != 1:
        raise ValueError("Qt source manifest must use schema 1")
    return data


def find_lupdate(program=None):
    if program:
        return program
    for name in ("lupdate6", "lupdate-qt6", "lupdate", "/usr/lib/qt6/bin/lupdate",
                 "/usr/lib64/qt6/bin/lupdate", "/opt/homebrew/opt/qt/bin/lupdate"):
        found = shutil.which(name)
        if found:
            return found
    raise ValueError("Qt Linguist lupdate was not found; install Qt6 Linguist tools")


def extract(root, lupdate=None, check=False):
    root = root.resolve()
    config = manifest(root)
    paths = []
    for relative in config["sources"]:
        directory = (root / relative).resolve()
        directory.relative_to(root)
        for path in directory.rglob("*"):
            if path.suffix not in (".cc", ".h", ".ui") or path.name.endswith("Doxy.h"):
                continue
            if {"examples", "build", "subprojects", ".git"}.intersection(path.relative_to(root).parts):
                continue
            path.resolve().relative_to(root)
            paths.append(path)
    with tempfile.TemporaryDirectory(prefix="gemc-qt-extract-") as temporary:
        output = Path(temporary) / "en.ts"
        result = subprocess.run([find_lupdate(lupdate), *map(str, sorted(paths)), "-no-obsolete",
                                 "-locations", "none", "-source-language", "en", "-target-language", "en",
                                 "-ts", str(output)], capture_output=True, text=True, cwd=root)
        if result.returncode or result.stderr.strip():
            raise ValueError(result.stderr.strip() or result.stdout.strip())
        _, messages = load(output)
    english = {key: (key[1],) * (plural_forms("en") if key[3] else 1) for key in messages}
    directory = root / "translations/gui"
    content = catalog_text("en", english, english)
    if check:
        if not (directory / "en.ts").exists() or load(directory / "en.ts")[1] != english:
            raise ValueError("English Qt catalog is stale; run qt_catalogs.py extract")
    else:
        (directory / "en.ts").write_text(content, encoding="utf-8")
        for language in config["languages"]:
            path = directory / (language + ".ts")
            if not path.exists():
                path.write_text(catalog_text(language, english, {}), encoding="utf-8")
    print(f"{'Checked' if check else 'Generated'} Qt English catalog: {len(english)} context/source messages.")
    return english


def validate(root):
    directory = root / "translations/gui"
    source_language, english = load(directory / "en.ts")
    if source_language != "en" or any(values is None or any(value != key[1] for value in values)
                                      for key, values in english.items()):
        raise ValueError("English Qt catalog must contain exact source text")
    for language in manifest(root)["languages"]:
        if not (directory / (language + ".ts")).is_file():
            raise ValueError(f"missing Qt catalog: {language}.ts")
    for path in sorted(directory.glob("*.ts")):
        language, messages = load(path)
        if language != path.stem or not re.fullmatch(r"[a-z]{2,3}(?:-[a-z0-9]+)*", language):
            raise ValueError(f"{path}: language must match its normalized filename")
        if messages.keys() - english.keys():
            raise ValueError(f"{path}: unknown Qt source/context messages")
        complete = sum(values is not None for values in messages.values())
        print(f"Qt coverage {language}: {complete}/{len(english)}; unfinished messages use English fallback.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("extract", "validate", "check"))
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent)
    parser.add_argument("--lupdate")
    args = parser.parse_args()
    try:
        if args.command != "validate":
            extract(args.root, args.lupdate, check=args.command == "check")
        if args.command != "extract":
            validate(args.root)
    except (ValueError, OSError, ET.ParseError, yaml.YAMLError) as error:
        parser.exit(1, f"error: {error}\n")


if __name__ == "__main__":
    main()

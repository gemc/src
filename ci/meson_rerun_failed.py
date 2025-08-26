#!/usr/bin/env python3
import json, os, shutil, subprocess, sys, glob

build   = sys.argv[1] if len(sys.argv) > 1 else 'build'
retries = int(sys.argv[2]) if len(sys.argv) > 2 else 2
np      = sys.argv[3] if len(sys.argv) > 3 else '1'
logbase = sys.argv[4] if len(sys.argv) > 4 else 'first-pass'   # set to 'meson-logs' to match your current output

logdir    = os.path.join(build, 'meson-logs')
json_path = os.path.join(logdir, f'{logbase}.json')
txt_path  = os.path.join(logdir, f'{logbase}.txt')

def run_all():
	# clean old logs for this logbase to avoid concatenation issues
	for p in (json_path, txt_path):
		try: os.remove(p)
		except FileNotFoundError: pass
	subprocess.check_call([
		'meson','test','-C',build,'--no-rebuild','-v',
		'--num-processes', np, '--print-errorlogs',
		'--logbase', logbase
	])

def find_json():
	if os.path.exists(json_path):
		return json_path
	# Fallback: try to find any single *.json produced by this run
	candidates = sorted(glob.glob(os.path.join(logdir, '*.json')))
	if len(candidates) == 1:
		return candidates[0]
	raise FileNotFoundError(
		f"Could not find {json_path}. Available JSONs: {candidates}"
	)

def load_concat_json(path):
	# Handle cases where Meson (or plugins) produce concatenated JSON docs
	with open(path, 'r', encoding='utf-8') as f:
		s = f.read().strip()
	dec = json.JSONDecoder()
	objs = []
	while s:
		s = s.lstrip()
		if not s: break
		obj, n = dec.raw_decode(s)
		objs.append(obj)
		s = s[n:]
	return objs[-1] if objs else {}

def failed_names(report):
	return [t['name'] for t in report.get('tests', []) if t.get('result') != 'OK']

# 1) First pass
run_all()

# 2) Read failures from the produced JSON
json_found = find_json()
backup     = os.path.join(logdir, f'{logbase}.first.json')
shutil.copy2(json_found, backup)
report = load_concat_json(backup)
fails  = failed_names(report)

# 3) Rerun failures serially, up to N retries
attempt = 0
while fails and attempt < retries:
	print(f"Re-running failed tests (round {attempt+1}): {fails}")
	for t in fails:
		subprocess.call([
			'meson','test','-C',build,'--no-rebuild','-v',
			'--num-processes','1','--print-errorlogs', t
		])
	# Optionally, re-check current failures by re-reading the latest JSON
	try:
		report = load_concat_json(find_json())
		fails  = failed_names(report)
	except Exception:
		# If no fresh JSON is present, leave 'fails' as-is for the next round
		pass
	attempt += 1

if fails:
	print("Still failing:", fails)
	sys.exit(1)

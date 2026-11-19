const assert = require('node:assert/strict');
const { test } = require('node:test');
const resolve = require('./pr_image_context.cjs');

function fixture() {
  const run = {
    repository: { id: 1 },
    path: '.github/workflows/pr-docker-image.yml',
    name: 'PR Docker Image',
    event: 'pull_request',
    status: 'completed',
    conclusion: 'success',
    run_attempt: 1,
    head_repository: { id: 2 },
    head_branch: 'contribution',
    head_sha: 'a'.repeat(40),
  };
  const pr = {
    number: 194,
    state: 'open',
    base: { repo: { id: 1 } },
    head: { repo: { id: 2 }, ref: 'contribution', sha: run.head_sha },
  };
  const pulls = [pr];
  const calls = [];
  const github = {
    rest: {
      actions: { getWorkflowRun: async args => {
        calls.push(['run', args]);
        return { data: run };
      } },
      pulls: { list: 'list', get: async args => {
        calls.push(['pr', args]);
        return { data: pr };
      } },
    },
    paginate: async () => pulls,
  };
  const context = {
    repo: { owner: 'gemc', repo: 'src' },
    payload: { repository: { id: 1 }, workflow_run: { id: 123, run_attempt: 1, pull_requests: [] } },
  };
  return { run, pr, pulls, calls, github, context, core: { info() {} } };
}

test('fork PR with an empty event PR array is associated through the API', async () => {
  const f = fixture();
  assert.equal(await resolve(f), 194);
  assert.deepEqual(f.calls, [['run', { owner: 'gemc', repo: 'src', run_id: 123 }]]);
});

test('same-repository PR is also allowed', async () => {
  const f = fixture();
  f.run.head_repository.id = f.pr.head.repo.id = 1;
  assert.equal(await resolve(f), 194);
});

const rejected = {
  'different workflow path': f => { f.run.path = '.github/workflows/test.yml'; },
  'different workflow name': f => { f.run.name = 'Test'; },
  'different repository': f => { f.run.repository.id = 9; },
  'push build': f => { f.run.event = 'push'; },
  'privileged PR build': f => { f.run.event = 'pull_request_target'; },
  'failed build': f => { f.run.conclusion = 'failure'; },
  'cancelled build': f => { f.run.conclusion = 'cancelled'; },
  'incomplete build': f => { f.run.status = 'in_progress'; },
  'superseded attempt': f => { f.run.run_attempt = 2; },
  'deleted fork': f => { f.run.head_repository = null; },
  'closed PR': f => { f.pr.state = 'closed'; },
  'stale commit': f => { f.pr.head.sha = 'b'.repeat(40); },
  'different fork with the same commit': f => { f.pr.head.repo.id = 9; },
  'different head branch': f => { f.pr.head.ref = 'other'; },
  'deleted PR head repository': f => { f.pr.head.repo = null; },
  'different PR base repository': f => { f.pr.base.repo.id = 9; },
  'non-numeric PR number': f => { f.pr.number = '194\nimage=other'; },
  'invalid PR number': f => { f.pr.number = -1; },
  'no associated open PR': f => { f.pulls.length = 0; },
  'ambiguous open PRs': f => { f.pulls.push({ ...f.pr, number: 195 }); },
};
for (const [name, mutate] of Object.entries(rejected)) {
  test(`rejects ${name}`, async () => {
    const f = fixture();
    mutate(f);
    assert.equal(await resolve(f), null);
  });
}

test('publication rechecks the selected PR directly after acquiring concurrency', async () => {
  const f = fixture();
  f.expectedPullRequest = '194';
  assert.equal(await resolve(f), 194);
  assert.deepEqual(f.calls[1], ['pr', { owner: 'gemc', repo: 'src', pull_number: 194 }]);
  f.pr.state = 'closed';
  assert.equal(await resolve(f), null);
  f.pr.state = 'open';
  f.pr.head.sha = 'b'.repeat(40);
  assert.equal(await resolve(f), null);
});

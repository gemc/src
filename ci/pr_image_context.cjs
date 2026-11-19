// Called only from the trusted publisher checkout, never from a PR checkout or artifact.
module.exports = async ({ github, context, core, expectedPullRequest }) => {
  const { owner, repo } = context.repo;
  const eventRun = context.payload.workflow_run;
  const { data: run } = await github.rest.actions.getWorkflowRun({
    owner, repo, run_id: eventRun.id,
  });
  if (run.repository.id !== context.payload.repository.id ||
      run.path !== '.github/workflows/pr-docker-image.yml' ||
      run.name !== 'PR Docker Image' || run.event !== 'pull_request' ||
      run.status !== 'completed' || run.conclusion !== 'success' ||
      run.run_attempt !== eventRun.run_attempt || !run.head_repository) {
    core.info('Skipping: not a successful current PR image build in this repository.');
    return null;
  }

  // Fork runs can have an empty workflow_run.pull_requests array. Query open PRs instead.
  const pulls = expectedPullRequest
    ? [(await github.rest.pulls.get({ owner, repo, pull_number: Number(expectedPullRequest) })).data]
    : await github.paginate(github.rest.pulls.list, { owner, repo, state: 'open', per_page: 100 });
  const matches = pulls.filter(pr =>
    Number.isSafeInteger(pr.number) && pr.number > 0 && pr.state === 'open' &&
    pr.base.repo.id === context.payload.repository.id &&
    pr.head.repo?.id === run.head_repository.id &&
    pr.head.ref === run.head_branch && pr.head.sha === run.head_sha
  );
  if (matches.length !== 1) {
    core.info('Skipping: build does not identify exactly one open PR at its current head.');
    return null;
  }
  return matches[0].number;
};

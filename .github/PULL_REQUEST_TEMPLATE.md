## PR checklist — please run through before marking ready

- [ ] I have updated documentation as needed (README, UPGRADING_CPP17.md)
- [ ] CI passes for the minimum supported toolchains (Linux / macOS / Windows smoke checks)
- [ ] If this change affects ABI, I have documented the changes and added notes to the PR
- [ ] If this PR adds/changes public headers or exported symbols, mention the impact and testing performed
- [ ] I have added unit tests for any important logic changes

If this PR touches ABI or packaging, add the label `abi-impact` so release tooling and reviewers are alerted.

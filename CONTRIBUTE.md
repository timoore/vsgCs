# vsgCs Development and Contributions

Contributions and bug fixes to vsgCs should be made by GitHub pull requests. Note that, by making a pull request, you agree to GitHub's terms of service and the license of the project.

vsgCs follows the [branching model](https://git-scm.com/docs/gitworkflows) used by the Git project itself. In this model, there at least two branches:
- `master` is the source of releases; at any given time, it should be more stable and have more features than the previous release.
- `next` contains new development that might eventually be included in a release.

Unlike other popular workflows, `next` is never merged directly to `master`. Development is done on topic branches that originate on `master`. Those branches are quickly merged to `next` and then, after they have been deemed to have "cooked" enough, are merged separately to `master`. The topic branches kind of take on a life of their own, with testing in combination with other branches in progress.

The history of `master` is an orderly progression of merges from topic branches. On the other hand, `next`'s history can get quite messy. Although `master` and `next` can be textually identical at times, their history diverges quickly. Periodically (right after a release), `next` is rewound to the current tip of `master` and the process starts again.

As a user of vsgCs, you should use `next` as much as you can in order to test new features. However, new development on vsgCs itself should never be based on `next`; it should be done on a topic branch based on `master`. It remains to be seen how GitHub PRs will translate into topic branches.


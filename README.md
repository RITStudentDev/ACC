# ACC
ACC is a HTTP container hosting service for self hosting web applications.

# Development workflow
## Branch naming conventions
Contributors are recommended to follow a naming convetion when creating new branches. The branch name should be in the following format.
```
<descriptor>/<focus>/<Username>
```
A descriptor is one word that is used to describe what is being done on the branch. The descriptor should be a member of the folowing list.
- dev
- feature
- update
- fix

The branch focus should be a very brief more specific description of what is being done on the branch where each word should be separated by hyphens. If the branch was made to fix a reported issue the focus should be the issue number.

Username should be the git username that was configured in local git settings.

## Commit messages
Commit messages should be a brief description of what was changed from the previous commit to the current commit. In the case of a commit needing to be done in order to resolve a conflict, state that this was the case in the commit message.

## Pull Requests
When making a pull request write an overall description covering all of the changes that the commits that were made on that branch. This is to make code review easier as the intended goal of the branch will be made clearer for the person reviewing.
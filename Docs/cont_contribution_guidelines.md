# Contributing to CARLA

The CARLA team is glad to accept contributions from anybody willing to collaborate. There are different ways to contribute to the project, depending on the capabilities of the contributor. The team will work as much as possible so that contributions are successfully integrated in CARLA.  

Take a look and don't hesitate!  

*   [__Report bugs__](#report-bugs)  
*   [__Request features__](#request-features)  
*   [__Code contributions__](#code-contributions)  
	*   [Learn about Unreal Engine](#learn-about-unreal-engine)  
	*   [Before getting started](#before-getting-started)  
	*   [Coding standard](#coding-standard)  
	*   [Submission](#submission)  
	*   [Checklist](#checklist)  
*   [__Art contributions__](#art-contributions)  
*   [__Docs contributions__](#docs-contributions)  

---
## Report bugs

Issues can be reported in the [issue section][issueslink] on GitHub. Before reporting a new bug, make sure to do some checkups.  

__1. Check if the bug has been reported.__ Look it up in that same issue section on GitHub.  

__2. Read the docs.__ Make sure that the issue is a bug, not a misunderstanding on how is CARLA supposed to work. Read the pages related to the issue in the [Documentation][docslink] and take a look at the [FAQ][faqlink] page.

[issueslink]: https://github.com/carla-simulator/carla/issues
[docslink]: http://carla.readthedocs.io
[faqlink]: build_faq.md

---
## Request features

Ideas for new features are also a great way to contribute. Any suggestion that could improve the users' experience  can be submitted in the corresponding GitHub section [here][frlink].

[frlink]: https://github.com/carla-simulator/carla/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+request%22+sort%3Acomments-desc

---
## Code contributions

Before starting hands-on on coding, please check out the [issue board][issueboard] to check what is the team already working on, to avoid overlapping. In case of doubt or to discuss how to proceed, please contact one of us (or send an email to <carla.simulator@gmail.com>).  

In order to start working, [fork the CARLA repository](https://docs.github.com/en/enterprise/2.13/user/articles/fork-a-repo), and clone said fork in your computer. Remember to [keep your fork in sync](https://docs.github.com/en/enterprise/2.13/user/articles/syncing-a-fork) with the original repository.  

[issueboard]: https://github.com/carla-simulator/carla/issues

### Learn about Unreal Engine

A basic introduction to C++ programming with UE4 can be found at Unreal's [C++ Programming Tutorials][ue4tutorials]. There are other options online, some of them not free of charge. The [Unreal C++ Course at Udemy][ue4course] it's pretty complete and there are usually offers that make it very affordable.

[ue4tutorials]: https://docs.unrealengine.com/latest/INT/Programming/Tutorials/
[ue4course]: https://www.udemy.com/unrealcourse/

### Before getting started

Check out the [CARLA Design](index.md)<!-- @todo --> document to get an idea on the different modules that compose CARLA. Choose the most appropriate one
to hold the new feature. Feel free to contact the team in the [Discord server](https://discord.com/invite/8kqACuC) in case any doubt arises during the process.  

### Coding standard

Follow the current [coding standard](cont_coding_standard.md) when submitting new code.  

### Submission

Contributions and new features are not merged directly to the `master` branch, but to an intermediate branch named `dev`. This [Gitflow](https://nvie.com/posts/a-successful-git-branching-model/) branching model makes it easier to maintain a stable master branch. This model requires a specific workflow for contributions.  

*   Always keep your `dev` branch updated with the lastest changes.  
*   Develop the contribution in child branch from `dev` named as `username/name_of_the_contribution`.  
*   Once the contribution is ready, submit a pull-request from your branch to `dev`. Try to be as descriptive as possible when filling the description. Note that there are some checks that the new code is required to pass before merging. The checks are automatically run by the continuous integration system. A green tick mark will appear if the checks are successful. If a red mark, please correct the code accordingly.  

Once the contribution is merged in `dev`, it can be tested with the rest of new features. By the time of the next release, the `dev` branch will be merged to `master`, and the contribution will be available and announced.  

### Checklist  

*   [ ] Your branch is up-to-date with the `dev` branch and tested with latest changes.  
*   [ ] Extended the README/documentation, if necessary.  
*   [ ] Code compiles correctly.  
*   [ ] All tests passing with `make check`.  

---
## Art contributions

Art contributions include vehicles, walkers, maps or any other type of assets to be used in CARLA. These are stored in a BitBucket repository, which has some account space limitations. For said reason, the contributor will have to get in touch with the CARLA team, and ask them to create a branch on the content repository for the contributions.  

__1. Create a BitBucket account.__ Visit the [Bitbucket page](https://bitbucket.org).  

__2. Contact the art team to get access to the content repository.__ Join the [Discord server](https://discord.com/invite/8kqACuC). Go to the __Contributors__ channel and request for access to the content repostory.  

__3. A branch will be created for each contributor.__ The branch will be named as `contributors/contributor_name`. All the contributions made by said user should be made in that corresponding branch.  

__4. Build CARLA.__ In order to contribute, a CARLA build is necessary. Follow the instructions to build either in [Linux](https://carla.readthedocs.io/en/latest/build_linux/) or [Windows](https://carla.readthedocs.io/en/latest/build_windows/).  

__5. Download the content repository.__ Follow the instructions to update the content in [here](https://carla.readthedocs.io/en/latest/build_update/#get-development-assets).

__6. Update the branch to be in sync with master.__ The branch should always be updated with the latest changes in master.  

__7. Upload the contribution.__ Do the corresponding changes and push the branch to origin.  

__8. Wait for the art team to check it up.__ Once the contribution is uploaded, the team will check that everything is prepared to be merged with master.  


---
## Docs contributions

If some documentation is missing, vague or imprecise, it can be reported as with any other bug (read the previous section on [how to report bugs](#report-bugs)). However, users can contribute by writing documentation.  

The documentation is written with a mix of [Markdown](https://www.markdownguide.org/) and HTML tags, with a some extra CSS code for features such as tables or the [town slider](https://carla.readthedocs.io/en/latest/core_map/#carla-maps). Follow the steps below to start writing documentation. 

!!! Important
    To submit docs contributions, follow the same workflow explained right above in [code contributions](#submission). To sum up, contributions are made in a child branch from `dev` and merged to said branch.  

__1. Build CARLA from source.__ Follow the steps in the docs to build on [Linux](build_linux.md) or [Windows](build_windows.md).  


__2. Install [MkDocs](http://www.mkdocs.org/).__ MkDocs is a static site generator used to build documentation. 

```sh
sudo pip install mkdocs
```

__3. Visualize the docs.__ In the main CARLA folder, run the following command and click the link that appears in the terminal (http://127.0.0.1:8000) to open a local visualization of the documentation.  

```sh
mkdocs serve
```
__4. Create a git branch.__ Make sure to be in the `dev` branch (updated to latest changes) when creating a new one. 

```sh
git checkout -b <contributor_name>/<branch_name>
```

__5. Write the docs.__ Edit the files following the guidelines in the [documentation standard](cont_doc_standard.md) page.  

__6. Submit the changes.__ Create a pull request in the GitHub repository, and add one of the suggested reviewers. Try to be as descriptive as possible when filling the pull-request description.  

__7. Wait for review.__ The team will check if everything is ready to be merged or any changes are needed.  

!!! Warning
    The local repository must be updated with the latest updates in the `dev` branch.  
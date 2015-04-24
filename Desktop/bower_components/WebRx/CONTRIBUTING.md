# Contributing

- Fork the repository
- Make one or more well commented and clean commits to the repository. You can make a new branch here if you are modifying more than one part or feature.
- Run the tests
- Submit a [pull request](https://help.github.com/articles/using-pull-requests/)

## Pull Requests

Your pull request should: 

* Include a description of what your change intends to do
* Be a child commit of a reasonably recent commit in the **master** branch 
    * Requests need not be a single commit, but should be a linear sequence of commits (i.e. no merge commits in your PR)
* It is desirable, but not necessary, for the tests to pass at each commit
* Have clear commit messages 
    * e.g. "Refactor feature", "Fix issue", "Add tests for issue"
* Include adequate tests 
    * At least one test should fail in the absence of your non-test code changes. If your PR does not match this criteria, please specify why
    * Tests should include reasonable permutations of the target fix/change
    * Include baseline changes with your change
    * All changed code must have 100% code coverage
* To avoid line ending issues, set `autocrlf = input` and `whitespace = cr-at-eol` in your git configuration

## Running the Tests

To run all tests, invoke the *test* target using grunt:

`grunt test`

This run will all tests; to run only a specific subset of tests, use:

`grunt test --filter=<regex>`

e.g. to run all binding tests:

`grunt test --filter=binding`

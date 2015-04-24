[![Build Status](https://ci.appveyor.com/api/projects/status/hm6sojygo41lbiln?svg=true)](https://ci.appveyor.com/project/webrxjs/webrx)
[![Bower version](https://img.shields.io/bower/v/WebRx.svg)](https://github.com/WebRxJS/WebRx)
[![NuGet version](https://img.shields.io/nuget/v/WebRx.svg)](https://www.nuget.org/packages/WebRx/)
[![Built with Grunt](https://cdn.gruntjs.com/builtwith.png)](http://gruntjs.com/)
<!-- [![Build Status](https://travis-ci.org/WebRxJS/WebRx.png)](https://travis-ci.org/WebRxJS/WebRx) -->

# WebRx

WebRx is a Javascript MVVM-Framework built on [ReactiveX for Javascript (RxJs)](http://reactivex.io) that combines functional-reactive programming with Observable-driven declarative Data-Binding, Templating and Client-Side Routing.

#### Features

- Tested with IE9+, Firefox 5+, Chrome 5+, Safari 5+, Android Browser 4.0+, iOS Safari 5.0+
- [Documentation](http://webrxjs.org/docs)
- Declarative One-way and Two-way data-binding with many built-in operators
- Supports self-contained, reusable *Components* modelled after the upcoming Web-Components standard
- Out-of-the box support for *Modules* to facilitate code-reuse and separation of concerns
- Integrated state-based routing engine inspired by Angular's [UI-Router](https://github.com/angular-ui/ui-router)
- No dependencies besides RxJS-Lite
- Compact (~25Kb minified & compressed)
- First class [TypeScript](http://www.typescriptlang.org/) support

#### Installation

- Installation via NuGet
```bash
PM> Install-Package WebRx
```

- Installation via Bower
```bash
bower install WebRx
```

- or download the [latest release as zip](http://webrxjs.org/downloads/web.rx.zip)

Make sure to include script-references to rx.lite.js and rx.lite.extras.js **before** web.rx.js when integrating WebRx into your projects.

#### Documentation

WebRx's documentation can be found on [here](http://webrxjs.org/docs).

#### Support 

Post your questions to [Stackoverflow](https://stackoverflow.com/questions/tagged/webrx) tagged with <code>webrx</code>.

#### Contributing

There are many ways to [contribute](https://github.com/oliverw/WebRx/blob/master/CONTRIBUTING.md) to WebRx.

* [Submit bugs](https://github.com/oliverw/WebRx/issues) and help us verify fixes as they are checked in.
* Review the [source code changes](https://github.com/oliverw/WebRx/pulls).
* Engage with other WebRx users and developers on [StackOverflow](http://stackoverflow.com/questions/tagged/webrx). 
* Join the [#webrx](http://twitter.com/#!/search/realtime/%23webrx) discussion on Twitter.
* [Contribute bug fixes](https://github.com/oliverw/WebRx/blob/master/CONTRIBUTING.md).


### License

MIT license - [http://www.opensource.org/licenses/mit-license.php](http://www.opensource.org/licenses/mit-license.php)

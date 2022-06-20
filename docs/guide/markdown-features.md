# Markdown Features

A document should be easy-to-read and easy-to-write.

## Document Format

A document is represented in Markdown format:

```markdown
# Title

The content goes here...
```

Internally we use the blazing-fast [marked](https://marked.js.org) to parse Markdown, so almost all [GitHub Flavored Markdown](https://github.github.com/gfm/) features are supported. 

Check out the introduction for [Markdown](https://daringfireball.net/projects/markdown/) if you're not sure what it is.

## Links

### Internal Links

Internal links are converted to `<router-link>` for SPA-style navigation.

__Input__:

```markdown
- [Home](/) <!-- Send the user to Homepage -->
- [Use Vue in Markdown](/guide/use-vue-in-markdown) <!-- Send the user to another page -->
- [Check out the `title` option](../options.md#title) <!-- Even relative link to markdown tile -->
```

__Output__:

- [Home](/) <!-- Send the user to Homepage -->
- [Use Vue in Markdown](/guide/use-vue-in-markdown) <!-- Send the user to another page -->
- [Check out the `title` option](../options.md#title) <!-- Even relative link to markdown tile -->


### External Links

External links automatically gets HTML attributes `target="_blank" rel="noopener noreferrer"`, for example:

__Input__:

```markdown
- [Docute website](https://docute.org)
- [Docute repo](https://github.com/egoist/docute)
```

__Output__:

- [Docute website](https://docute.org)
- [Docute repo](https://github.com/egoist/docute)

## Task List

__Input__:

```markdown
- [ ] Rule the web
- [x] Conquer the world
- [ ] Learn Docute
```

__Output__:

- [ ] Rule the web
- [x] Conquer the world
- [ ] Learn Docute

## Code Highlighting

Code fences will be highlighted using [Prism.js](https://prismjs.com/), example code:

```js
// Returns a function, that, as long as it continues to be invoked, will not
// be triggered. The function will be called after it stops being called for
// N milliseconds. If `immediate` is passed, trigger the function on the
// leading edge, instead of the trailing.
function debounce(func, wait, immediate) {
	var timeout;
	return function() {
		var context = this, args = arguments;
		var later = function() {
			timeout = null;
			if (!immediate) func.apply(context, args);
		};
		var callNow = immediate && !timeout;
		clearTimeout(timeout);
		timeout = setTimeout(later, wait);
		if (callNow) func.apply(context, args);
	};
};
```

The languages we support by default:

<ul>
  <li v-for="lang in builtinLanguages" :key="lang">
    {{ lang }}
  </li>
</ul>

You can use [highlight](/options#highlight) option to add more languages.

## Code Fence Options

Next to the code fence language, you can use a JS object to specify options:

````markdown
```js {highlightLines: [2]}
function foo() {
  console.log('foo')
}
```
````

Available options:

- `highlightLines`: [Line Highlighting in Code Fences](#line-highlighting-in-code-fences)
- `mixin`: [Adding Vue Mixin](#adding-vue-mixin)

## Line Highlighting in Code Fences

__Input:__

````markdown
```js {highlight:[3,'5-7',12]}
class SkinnedMesh extends THREE.Mesh {
  constructor(geometry, materials) {
    super(geometry, materials);

    this.idMatrix = SkinnedMesh.defaultMatrix();
    this.bones = [];
    this.boneMatrices = [];
    //...
  }
  update(camera) {
    //...
    super.update();
  }
  static defaultMatrix() {
    return new THREE.Matrix4();
  }
}
```
````

__Output:__

```js {highlight:[3,'5-7',12]}
class SkinnedMesh extends THREE.Mesh {
  constructor(geometry, materials) {
    super(geometry, materials);

    this.idMatrix = SkinnedMesh.defaultMatrix();
    this.bones = [];
    this.boneMatrices = [];
    //...
  }
  update(camera) {
    //...
    super.update();
  }
  static defaultMatrix() {
    return new THREE.Matrix4();
  }
}
```

## Adding Vue Mixin

Adding a Vue mixin to the Markdown component:

````markdown
<button @click="count++">{{ count }}</button> people love Docute.

```js {mixin:true}
{
  data() {
    return {
      count: 1000
    }
  }
}
```
````

<button @click="count++">{{ count }}</button> people love Docute.

```js {mixin:true}
{
  data() {
    return {
      count: 1000
    }
  }
}
```

## Using Mermaid

[Mermaid](https://mermaidjs.github.io/) is a way to write charts in plain text, you can use a simple Docute plugin to add Mermaid support:

```html
<script src="https://unpkg.com/docute@4/dist/docute.js"></script>
<!-- Load mermaid -->
<script src="https://unpkg.com/mermaid@8.0.0-rc.8/dist/mermaid.min.js"></script>
<!-- Load the mermaid plugin -->
<script src="https://unpkg.com/docute-mermaid@1/dist/index.min.js"></script>

<!-- Use the plugin -->
<script>
new Docute({
  // ...
  plugins: [
    docuteMermaid()
  ]
})
</script>
```

## HTML in Markdown

You can write HTML in Markdown, for example:

```markdown
__FAQ__:

<details><summary>what is the meaning of life?</summary>

some say it is __42__, some say it is still unknown.
</details>
```

__FAQ__:

<details><summary>what is the meaning of life?</summary>

some say it is __42__, some say it is still unknown.
</details>

<br>

<Note>In fact you can even use Vue directives and Vue components in Markdown too, learn more about it [here](./use-vue-in-markdown.md).</Note>

# 配置项

用于 `Docute` 构造函数的配置项。

```js
new Docute(options)
```


## target

- Type: `string`
- Default: `docute`

目标元素的 ID，会被用于创建 app，比如 `app` 或者 `#app`。

## title

- 类型：`string`
- 默认值：`document.title`

网站标题。

## logo

- Type: `string` `object`
- Default: `<span>{{ $store.getters.config.title }}</span>`

Customize the logo in header.

- `string`: Used as Vue template.
- `object`: Used as Vue component.

## nav

- 类型: `Array<NavItem>`

在头部显示的导航栏。

```ts
interface NavItem {
  title: string
  link?: string
  // Whether to open the link in a new tab
  // Only works for external links
  // Defaults to `true`
  openInNewTab?: boolean
  // Use `children` instead of `link` to display dropdown
  children?: Array<NavItemLink>
}

interface NavItemLink {
  title: string
  link: string
  openInNewTab?: boolean
}
```

## sidebar

- 类型：`Array<SidebarItem>` `(store: Vuex.Store) => Array<SidebarItem>`

在侧边栏中显示的导航栏。

```ts
type SidebarItem = SingleItem | MultiItem

interface SingleItem {
  title: string
  link: string
  // Whether to open the link in a new tab
  // Only works for external links
  // Defaults to `true`
  openInNewTab?: boolean
}

interface MultiItem {
  title: string
  children: Array<SingleItem>
  /**
   * Whether to show TOC
   * Default to `true`
   */
  toc?: boolean
  /**
   * Whether to make children collapsable
   * Default to `true` 
   */
  collapsable?: boolean
}
```

## sourcePath

- 类型：`string`
- 默认值：`'./'`

从 source path 获取 markdown 文件，默认情况下，我们从 `index.html` 所在的目录加载它们。

它也可以是完整的 URL，例如： `https://some-website/path/to/markdown/files`，以便于你可以从其他域名加载文件。


## routes

- Type: `Routes`

Use this option to make Docute fetch specific file or use given content for a path.

```ts
interface Routes {
  [path: string]: RouteData
}

interface RouteData {
  /* Default to the content h1 header */
  title?: string
  /* One of `content` and `file` is required */
  content?: string
  /* Response will be used as `content` */
  file?: string
  /* Parse the content as markdown, true by default */
  markdown?: boolean
  [k: string]?: any
}
```

## componentMixins

- 类型: `Array<Object>`

一个包含 [Vue mixins](https://vuejs.org/v2/api/#mixins) 的数组，会被应用到所有的 Markdown 组件中。

## highlight

- 类型：`Array<string>`

需要语法高亮的语言名称数组。查阅 [Prism.js](https://unpkg.com/prismjs/components/) 获取所有支持的语言名称。（不需要携带 `prism-` 前缀)。

例如：`highlight: ['typescript', 'go', 'graphql']`。

## editLinkBase

- 类型：`string`

*编辑链接*的 URL 基础路径。

例如，如果将 markdown 文件存储在 Github 仓库的 master 分支的 `docs` 文件夹中，那么该路径应该为：

```
https://github.com/USER/REPO/blob/master/docs
```

## editLinkText

- 类型：`string`
- 默认值：`'Edit this page'`

*编辑链接*的文字内容。

# theme

- 类型: `string`
- 默认值: `default`
- 内置项: `default` `dark`

网站主题。

## detectSystemDarkTheme

- Type: `boolean`
- Default: `undefined`

In recent versions of macOS (Mojave) and Windows 10, users have been able to enable a system level dark mode. Set this option to `true` so that Docute will use the dark theme by default if your system has it enabled.

## darkThemeToggler

- Type: `boolean`
- Default: `undefined`


显示一个按钮用于切换夜间主题。

## layout

- 类型: `string`
- 默认值: `wide`
- 可选项: `wide` `narrow` `left`

网站布局。

## versions

- 类型: `Versions`

设置此项之后, Docute 会在侧边栏显示一个版本选择器。

```ts
interface Versions {
  // 版本号, 比如 `v1`
  [version: string]: {
    // 指向相关文档的链接
    link: string
  }
}
```

## cssVariables

- Type: `object` `(theme: string) => object`

Override CSS variables.

## overrides

- 类型：`{[path: string]: LocaleOptions}`

```ts
interface LocaleOptions extends Options {
  language: string
}
```

## router

- Type: `ConstructionOptions`

All vue-router's [Construction options](https://router.vuejs.org/api/#router-construction-options) except for `routes` are accepted here.

For example, you can set `router: { mode: 'history' }` to [get rid of the hash](https://router.vuejs.org/guide/essentials/history-mode.html#example-server-configurations) in URLs.

## banner / footer

- Type: `string` `VueComponent`

Display banner and footer. A string will be wrapped inside `<div class="docute-banner">` or `<div class="docute-footer">` and used as Vue template.

For example:

```js
new Docute({
  banner: `Please <a href="https://donate.com/link">
  donate</a> <ExternalLinkIcon /> to support this project!`
})
```

You can also use a Vue component:

```js
new Docute({
  banner: {
    template: `
    <div class="docute-banner">
      Please <a href="https://donate.com/link">
      donate</a> <ExternalLinkIcon /> to support this project!
    </div>
    `
  }
})
```

## imageZoom

- Type: `boolean`
- Default: `undefined`

Enable Medium-like image zoom effect to all images.

Alternatively you can use the [`<image-zoom>`](./builtin-components.md#imagezoom) component if you only need this in specific images.

## fetchOptions

- Type: `object`

The option for [`window.fetch`](https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API/Using_Fetch).

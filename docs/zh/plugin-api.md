# 插件 API

插件属性：

- `name`：`string` 插件名称.
- `extend(api: PluginAPI)`：扩展核心功能

## api.processMarkdown(fn)

- `fn`：`(text: string) => string | Promise <string>`

处理 markdown。

## api.extendMarkdownComponent(fn)

- `fn`: `(Component: VueComponentOptions) => void`

修改编译后的 Markdown 组件。

## api.processHTML(fn)

- `fn`：`(html: string) => string | Promise <string>`

处理 HTML.

## api.extendMarkedRenderer(fn)

- `fn`：`(renderer: marked.Renderer) => void`

你可以使用 `fn` 来修改我们使用的 [marked 渲染器](https://marked.js.org/#/USING_PRO.md#renderer)。

## api.onContentUpdated(fn)

- `fn`：`(vm: Vue) => void`

更新页面内容时将调用 `fn`。

## api.registerComponent(position, component)

- `position`: `string`
- `component`: `VueComponent`

Register a component at specific position:

- `sidebar:start`: The start of sidebar.
- `sidebar:end`: The end of sidebar.
- `content:start`: The start of page content.
- `content:end`: The end of page content.
- `header-right:start`: The start of right nav in site header.
- `header-right:end`: The end of right nav in site header.

## api.enableSearch(options)

Enable search bar.

Properties in `options`:

|Property|Type|Description|
|---|---|---|
|`handler`|`Handler`|A handler function triggered by every user input.|

```ts
type Handler = (keyword: string) => SearchResult[] | Promise<SearchResult[]>

interface SearchResult {
  title: string
  link: string
  label: string?
  description: string?
}
```

## api.router

基本上是 [Vue Router](https://router.vuejs.org/api/#router-instance-properties) 实例。

## api.store

基本上是 [Vuex](https://vuex.vuejs.org/api/#vuex-store-instance-properties) 实例。

# Plugin API

Plugin properties:

- `name`: `string` Plugin name.
- `extend(api: PluginAPI)`: Extending core features.

## api.processMarkdown(fn)

- `fn`: `(text: string) => string | Promise<string>`

Process markdown.

## api.processHTML(fn)

- `fn`: `(html: string) => string | Promise <string>`

Process HTML.

## api.extendMarkedRenderer(fn)

- `fn`: `(renderer: marked.Renderer) => void`

You can use `fn` to modify the [marked renderer](https://marked.js.org/#/USING_PRO.md#renderer) we use.

## api.extendMarkdownComponent(fn)

- `fn`: `(Component: VueComponentOptions) => void`

You can use this hook the modify the compiled markdown component.

## api.onContentUpdated(fn)

- `fn`: `(vm: Vue) => void`

`fn` will be called when the page content is updated.

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

Basically the [Vue Router](https://router.vuejs.org/api/#router-instance-properties) instance.

## api.store

Basically the [Vuex](https://vuex.vuejs.org/api/#vuex-store-instance-properties) instance.

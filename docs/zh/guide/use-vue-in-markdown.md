# 在 Markdown 中使用 Vue

在编写 Markdown 文档时可以充分利用 Vue 和 JavaScript 的强大功能！

## 插值（Interpolation）

每个 Markdown 文件首先会编译为 HTML，然后渲染为 Vue 组件这意味着你可以在文本中使用 Vue 式的插值：

__输入__:

```markdown
{{ 1 + 1 }}
```

__输出__:

```
2
```

## Escaping

如果要在文本中禁用 Vue 式插值，可以将其包装在代码块或内联代码中，如下所示：

__输入__:

````markdown
```js
const foo = `{{ 安全，这不会被插值！}}`
```

`{{ bar }}` 也是安全的！
````

__输出__:

```js
const foo = `{{ 安全，这不会被插值！}}`
```

`{{ bar }}` 也是安全的！

## 使用组件

Docute 在 `window` 对象上暴露了 `Vue` 的构造函数，因此你可以使用它来注册全局组件，以便于在 Markdown 文档中使用：

```js {highlight:['6-13']}
Vue.component('ReverseText', {
  props: ['text'],
  template: `
    <div class="reverse-text">
      {{ reversedText }}
      <v-style>
      .reverse-text {
        border: 1px solid var(--border-color);
        padding: 20px;
        font-weight: bold;
        border-radius: 4px;
      }
      </v-style>
    </div>
  `,
  computed: {
    reversedText() {
      return this.text.split('').reverse().join('')
    }
  }
})
```

你可能会注意到高亮的部分，由于你不能直接在 Vue template 里使用 `style` 元素，于是我们提供了 `v-style` 组件来作为替代。类似地，我们也提供了 `v-script` 组件。

__输入__:

```markdown
<ReverseText text="hello world" />
```

__输出__:

<ReverseText text="hello world" />


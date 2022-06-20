# 内置组件

Docute 附带一些内置的 Vue 组件。

## `<ImageZoom>`

使用与 Medium 相同的缩放效果显示 image 。

|属性|类型|默认值|描述|
|---|---|---|---|
|url|`string`|N/A|Image 的 URL|
| title  | `string`  | N/A     | Image title             |
|alt|`string`|N/A|占位文字|
|border|`boolean`|`false`|是否显示图像周围的边框|
|width|`string`|N/A|Image 宽度|

示例：

```markdown
<ImageZoom 
  src="https://i.loli.net/2018/09/24/5ba8e878850e9.png" 
  :border="true" 
  width="300"
/>
```

<ImageZoom src="https://i.loli.net/2018/09/24/5ba8e878850e9.png" :border="true" width="300"/>


## `<Badge>`

A small count and labeling component.

| Prop     | Type                                                | Default | Description             |
| -------- | --------------------------------------------------- | ------- | ----------------------- |
| type     | <code>'tip' &#x7C; 'success' &#x7C; 'warning' &#x7C; 'danger'</code> | N/A | Badge type              |
| color    | `string`                                            | N/A     | Custom background color |
| children | `string`                                            | N/A     | Badge text              |

<br>

Example:

```markdown
- Feature 1 <Badge>Badge</Badge>
- Feature 2 <Badge type="tip">Tip</Badge>
- Feature 3 <Badge type="success">Success</Badge>
- Feature 4 <Badge type="warning">Warning</Badge>
- Feature 5 <Badge type="danger">Danger</Badge>
- Feature 6 <Badge color="magenta">Custom Color</Badge>
```

- Feature 1 <Badge>Badge</Badge>
- Feature 2 <Badge type="tip">Tip</Badge>
- Feature 3 <Badge type="success">Success</Badge>
- Feature 4 <Badge type="warning">Warning</Badge>
- Feature 5 <Badge type="danger">Danger</Badge>
- Feature 6 <Badge color="magenta">Custom Color</Badge>

## `<Note>`

Colored note blocks, to emphasize part of your page.

| Prop     | Type                                                                | Default             | Description                                       |
| -------- | ------------------------------------------------------------------- | ------------------- | ------------------------------------------------- |
| type     | <code>'tip' &#x7C; 'warning' &#x7C; 'danger' &#x7C; 'success'</code> | N/A                 | Note type                                         |
| label    | `string` `boolean`                                                  | The value of `type` | Custom note label text, use `false` to hide label |
| children | `string`                                                            | N/A                 | Note content                                      |

<br>

Examples:

```markdown
<Note>

This is a note that details something important.<br>
[A link to helpful information.](https://docute.org)

</Note>

<!-- Tip Note -->
<Note type="tip">

This is a tip for something that is possible.

</Note>

<!-- Warning Note -->
<Note type="warning">

This is a warning for something very important.

</Note>

<!-- Danger Note -->
<Note type="danger">

This is a danger for something to take action for.

</Note>
```

<Note>

This is a note that details something important.<br>
[A link to helpful information.](https://docute.org)

</Note>

<!-- Tip Note -->
<Note type="tip">

This is a tip for something that is possible.

</Note>

<!-- Warning Note -->
<Note type="warning">

This is a warning for something very important.

</Note>

<!-- Danger Note -->
<Note type="danger">

This is a danger for something to take action for.

</Note>

## `<Gist>`

Embed [GitHub Gist](https://gist.github.com/) into your Markdown documents.

|Prop|Type|Default|Description|
|---|---|---|---|
|id|`string`|N/A|Gist ID|

Example:

```markdown
<Gist id="71b8002ecd62a68fa7d7ee52011b2c7c" />
```

<Gist id="71b8002ecd62a68fa7d7ee52011b2c7c" />

## `<docute-select>`

A customized `<select>` component:

<!-- prettier-ignore -->
````vue
<docute-select :value="favoriteFruit" @change="handleChange">
  <option value="apple" :selected="favoriteFruit === 'apple'">Apple</option>
  <option value="banana" :selected="favoriteFruit === 'banana'">Banana</option>
  <option value="watermelon" :selected="favoriteFruit === 'watermelon'">Watermelon</option>
</docute-select>

Your favorite fruit: {{ favoriteFruit }}

```js {mixin: true}
module.exports = { 
  data() { 
    return { 
      favoriteFruit: 'banana' 
    }
  }, 
  methods: {
    handleChange(value) { 
      this.favoriteFruit = value
    } 
  }
}
```
````

<docute-select @change="handleChange" :value="favoriteFruit">
  <option value="apple" :selected="favoriteFruit === 'apple'">Apple</option>
  <option value="banana" :selected="favoriteFruit === 'banana'">Banana</option>
  <option value="watermelon" :selected="favoriteFruit === 'watermelon'">Watermelon</option>
</docute-select>

Your favorite fruit: {{ favoriteFruit }}

```js {mixin: true}
{
  data() {
    return {
      favoriteFruit: 'banana'
    }
  },
  methods: {
    handleChange(value) {
      this.favoriteFruit = value
    }
  }
}
```

## `<v-style>` `<v-script>`

在 Vue template 中替代 `<style>` 和 `<script>` 标签。

通常，在 Markdown 里你并不需要直接使用此组件。因为我们会自动将 markdown 中的 `<style>` 和 `<script>` 标签转换为该组件。

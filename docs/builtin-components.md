# Built-in Components

Docute comes with a set of built-in Vue components.

## `<ImageZoom>`

Use medium-style zoom effect to display certain image.

| Prop   | Type      | Default | Description              |
| ------ | --------- | ------- | ------------------------ |
| src    | `string`  | N/A     | URL to image             |
| title  | `string`  | N/A     | Image title             |
| alt    | `string`  | N/A     | Placeholder text         |
| border | `boolean` | `false` | Show border around image |
| width  | `string`  | N/A     | Image width              |

<br>

Example:

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

| Prop     | Type                                                                 | Default | Description             |
| -------- | -------------------------------------------------------------------- | ------- | ----------------------- |
| type     | <code>'tip' &#x7C; 'success' &#x7C; 'warning' &#x7C; 'danger'</code> | N/A     | Badge type              |
| color    | `string`                                                             | N/A     | Custom background color |
| children | `string`                                                             | N/A     | Badge text              |

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

| Prop     | Type                                                                 | Default             | Description                                       |
| -------- | -------------------------------------------------------------------- | ------------------- | ------------------------------------------------- |
| type     | <code>'tip' &#x7C; 'warning' &#x7C; 'danger' &#x7C; 'success'</code> | N/A                 | Note type                                         |
| label    | `string` `boolean`                                                   | The value of `type` | Custom note label text, use `false` to hide label |
| children | `string`                                                             | N/A                 | Note content                                      |

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

| Prop | Type     | Default | Description |
| ---- | -------- | ------- | ----------- |
| id   | `string` | N/A     | Gist ID     |

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

A hack for using `<style>` and `<script>` tags Vue template.

In general you don't need to use them directly, since we automatically convert `<style>` and `<script>` tags in Markdown to these components.

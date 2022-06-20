# Use Vue in Markdown

Leverage the power of Vue and JavaScript in writing Markdown document!

## Interpolation

Each markdown file is first compiled into HTML and then rendered as a Vue component. This means you can use Vue-style interpolation in text:

__Input__:

```markdown
{{ 1 + 1 }}
```

__Output__:

```
2
```

## Escaping

If you want to disable Vue-style interpolation in text, you can wrap it inside code fence or inline code as follows:

__Input__:

````markdown
```js
const foo = `{{ safe, this won't be interpolated! }}`
```

And `{{ bar }}` is safe too!
````

__Output__:

```js
const foo = `{{ safe, this won't be interpolated! }}`
```

And `{{ bar }}` is safe too!

## Using Components

Docute exposed the `Vue` constructor on `window` object, so you can use it to register global components in order to use in your Markdown document:

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

You may notice the highlighted part, since you can't directly use `style` tags in Vue template, here we provided the `v-style` component to work around that. Similarly there's also `v-script` component.

__Input__:

```markdown
<ReverseText text="hello world" />
```

__Output__:

<ReverseText text="hello world" />


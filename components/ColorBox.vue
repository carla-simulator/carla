<template>
  <span class="color-box" :style="{background:color}"></span>
</template>

<script>
export default {
  props: ['color']
}
</script>


<style scoped>
.color-box {
  height: 12px;
  width: 12px;
  display: inline-block;
  border: 1px solid #333;
}
</style>

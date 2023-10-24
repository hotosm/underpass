## Install Underpass UI

```sh
yarn add https://github.com/hotosm/underpass-ui.git
```

### Import styles and the UnderpassMap module

```js
import "@hotosm/underpass-ui/dist/index.css";
import { UnderpassMap } from "@hotosm/underpass-ui";
```

### Example

```jsx
<UnderpassMap
    center={[44.39016, -89.79617].reverse()}
    tags={"building=yes"}
    zoom={17}
/>
```


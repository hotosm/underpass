import './theme.css';

function cssValue(property) {
  return getComputedStyle(document.documentElement)
    .getPropertyValue(property);
}

export default function HOTTheme() {
  const colors = {
    primary: cssValue("--hottheme-color-primary"),
    secondary: cssValue("--hottheme-color-secondary"),
  };

  return {
    colors: colors,
    map: {
      waysFill: {
        'fill-opacity': .2,
      },
      waysLine: {
        'line-width': 1.5
      }
    }
  }
}

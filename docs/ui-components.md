# Underpass UI components

## Requirements

First, install and run the [Underpass Python REST API](https://github.com/hotosm/underpass/blob/master/docs/python-rest-api.md)

Then, for running the playground, you'll need:

* Node
* NPM
* Yarn

If you have problems installing Yarn, try with this approach:

```sh
npm install -g n
n stable
npm install --global yarn
```

## Run

```sh
git clone https://github.com/hotosm/underpass-ui.git
cd underpass-ui
yarn install
yarn cosmos
```

The default URL for the Underpass Python REST API is `http://localhost:8000`, but you can change it 
using an environment variable:

```sh
REACT_APP_UNDERPASS_API=http:://underpass.live:8000 yarn cosmos
```

## UnderpassMap component

This component shows a map updated with the latest changes made into OSM and the option to highlight certain aspects of data.

### Basic view

```js
<UnderpassMap 
	center={[-0.74293, -90.31972]}
/>
```

It will show a map with all buildings highlighted, as “building” is the default tag key.

<img width="689" alt="Screenshot 2023-07-13 at 21 20 22" src="https://github.com/hotosm/underpass/assets/1226194/a9ce1a9c-d5ae-4205-a7f8-927e6ebe60df">


If you want to highlight other features, like “amenity=hospital”, just pass the “tagKey” and "tagValue" properties:

```js
<UnderpassMap 
	center={[-0.74293, -90.31972]}
	tagKey="amenity"
	tagValue="hospital"
/>
```

<img width="694" alt="Screenshot 2023-07-13 at 21 20 33" src="https://github.com/hotosm/underpass/assets/1226194/c5b62e9d-4d34-442e-9726-8477cac2c59f">

### Data Quality

Use the property “highlightDataQualityIssues” to highlight features suspicious of having some issue with quality.

```js
<UnderpassMap 
	center={[-0.74293, -90.31972]}
	highlightDataQualityIssues
/>
```
<img width="694" alt="Screenshot 2023-07-13 at 21 20 53" src="https://github.com/hotosm/underpass/assets/1226194/82ae40e9-8ca5-4ab5-b866-41182b942f15">




# Underpass UI components

You can run the components playground following these steps.

## Requirements

First, install and run the [Install & run the Underpass Python REST API](https://github.com/hotosm/underpass/blob/master/docs)

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
cd js
yarn install
yarn cosmos
```

The default URL for the Underpass Python REST API is `http://localhost:8000`, but you can change it 
using an environment variable:

```sh
REACT_APP_UNDERPASS_API=http:://underpass.live:8000 yarn cosmos
```
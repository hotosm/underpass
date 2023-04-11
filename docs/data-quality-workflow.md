# How to implement a data quality workflow using Underpass and the iD editor

## 1. Capturing changes in the iD Editor before upload changes to OSM

You can see in this commit how it's possible to capture changes made to the map in an instance of the iD Editor inside Tasking Manager, just before uploading changes to the map:

https://github.com/hotosm/tasking-manager/compare/develop...feature/idEditorOnEnterSave

The new OnEnterSave property will be fired when the user enter to editor's "Upload" screen. This function will receive the changes in JXON format and this can be translated later to a OsmChange XML.

## 2. Analyze the OsmChange and display a data  quality review

### Python backend

Having Underpass built on a server (see https://github.com/hotosm/underpass/blob/master/doc/getting-started.md) you can install the Python bindings with `make install-python` 

After this step, you'll be able to analyze a OsmChange using Underpass, with Python code:

```python
import underpass as  u

validator = u.Validate()
return  json.loads(validator.checkOsmChange(
   osmchange, # XML OsmChange
   check      # Example: 'building'
)
```

The result will be a JSON message with the results of the validation.

### React front-end

There's an [Underpass React component](https://github.com/hotosm/underpass/tree/master/util/react/src/components/Underpass/ValidateOsmChange) that you can use for sending the OsmChange's XML received from the iD Editor to the API, then receive the JSON message with the results of the validation and display the results on the screen.

This component is very easy to use, just copy and import the React component:

```js
import { ValidateOsmChange } from  '../components/Underpass/ValidateOsmChange';
```
And then use it in your React app:

```jsx
<ValidateOsmChange osmchange={osmChange} />
```

The result will be a data quality review of the OsmChange, that you can perform and display just before uploading changes to the map.

![Screen Shot 2023-03-03 at 14 10 52](https://user-images.githubusercontent.com/1226194/222783929-0f596404-5b0d-4e77-93cd-d0feaf819b12.png)


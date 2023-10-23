## Call Underpass functions using Python C++ binding

### Import Underpass package

```py
    import underpass as u
```

### Validate OSM Change

```py
    with open("building.osc", 'r') as file:
        data = file.read().rstrip()

    validator = u.Validate()
    result = validator.checkOsmChange(data, "building")
```


import underpass as u

validator = u.Validate()

# Empty node
node = u.OsmNode()

# No tags
validation = validator.checkPOI(node, "building")
print(validation.dumpJSON())

print("")

# Complete
node.addTag("building", "yes")
validation = validator.checkPOI(node, "building")
print(validation.dumpJSON())

print("")

# Empty node
node = u.OsmNode()

# Incomplete
node.addTag("place", "city")
validation = validator.checkPOI(node, "place")
print(validation.dumpJSON())

print("")

# Complete
node.addTag("name", "Electric City")
validation = validator.checkPOI(node, "place")
print(validation.dumpJSON())

# Bad value
node.addTag("building", "yes")
validation = validator.checkPOI(node, "place")
print(validation.dumpJSON())

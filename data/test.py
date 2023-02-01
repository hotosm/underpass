import underpass as u

node = u.OsmNode()
node.id = 11111
node.change_id = 22222
node.dump()
print("- - - ")
v = u.Validate()
res = v._checkPOI(node, "building")
# print(res.hasStatus("incomplete"))
res.dump()

node.addTag("building", "yes")
res = v._checkPOI(node, "building")
res.dump()

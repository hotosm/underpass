/*
This query deletes entries created from OsmChanges,
closed 1 day back or before , that has no corresponding Changeset.

We run this query every 24 hours for cleaning the database
when running the replicator process with areaFilter disabled 
for osmchanges (--osmnoboundary).
*/
DELETE FROM changesets WHERE bbox is null and closed_at < NOW() - INTERVAL '24 HOURS' RETURNING id;

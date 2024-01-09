
def tagsQueryFilter(tagsQuery, table):
    query = ""
    tags = tagsQuery.split(",")
    keyValue = tags[0].split("=")

    if len(keyValue) == 2:
        query += "{0}.tags->>'{1}' ~* '^{2}'".format(table, keyValue[0], keyValue[1])
    else:
        query += "{0}.tags->>'{1}' IS NOT NULL".format(table, keyValue[0])

    for tag in tags[1:]:
        keyValue = tag.split("=")
        if len(keyValue) == 2:
            query += "OR {0}.tags->>'{1}' ~* '^{2}'".format(table, keyValue[0], keyValue[1])
        else:
            query += "OR {0}.tags->>'{1}' IS NOT NULL".format(table, keyValue[0])
    return query

def hashtagQueryFilter(hashtag, table):
    return table + ".changeset IN (SELECT c.id FROM changesets c where jsonb_path_exists(to_jsonb(hashtags), '$[*] ? (@ like_regex \"^{0}\")') GROUP BY C.id)".format(hashtag)

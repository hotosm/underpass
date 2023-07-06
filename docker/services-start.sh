docker exec -t underpass tmux new-session -d -s replicator 'cd /code/build && ./underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")' && \
docker exec -t underpass tmux new-session -d -s rest-api 'cd /code/util/python/restapi && uvicorn main:app --reload --host 0.0.0.0' && \
docker exec -t underpass tmux new-session -d -s react-cosmos 'cd /code/util/react && REACT_APP_CENTER=$(psql underpass -c "SELECT ST_X(geometry)::text || ',' || ST_Y(geometry)::text as coords from raw_node limit 1;" | grep ,) yarn cosmos' && \
echo "Services started."

docker exec -t underpass tmux new-session -d -s replicator 'cd /code/build && ./replicator -t $(date +%Y-%m-%dT%H:%M:%S -d "1 week ago")' && \
docker exec -t underpass tmux new-session -d -s rest-api 'cd /code/util/python/restapi && uvicorn main:app --reload --host 0.0.0.0' && \
docker exec -t underpass tmux new-session -d -s react-cosmos 'cd /code/util/react && yarn cosmos' && \
echo "Services started."

docker exec -t underpass tmux kill-session -t replicator && \
docker exec -t underpass tmux kill-session -t rest-api && \
docker exec -t underpass tmux kill-session -t react-cosmos && \
echo "Services stopped."

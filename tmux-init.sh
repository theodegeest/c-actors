#!/bin/bash

# Check if tmux is running
if [ -z "$TMUX" ]; then
    echo "This script must be run inside a tmux session."
    exit 1
fi

tmux rename-window -t 1 'code'
tmux send-keys -t 1 C-l
tmux send-keys -t 1 "exa -T -L 1" C-m

tmux new-window -n 'build'
tmux send-keys -t 2 "cd ./build/" C-m
tmux send-keys -t 2 C-l
tmux send-keys -t 2 "exa -T -L 1" C-m

tmux select-window -t 1

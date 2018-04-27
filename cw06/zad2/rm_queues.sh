echo "Before removing:"
ipcs
echo "Removing queues..."
ipcrm --all=msg
echo "After removing:"
ipcs

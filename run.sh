sudo mount -t debugfs none /sys/kernel/debug
sudo sh -c 'echo 1 > /sys/kernel/debug/tracing/tracing_on'
sudo ./ecli run package.json

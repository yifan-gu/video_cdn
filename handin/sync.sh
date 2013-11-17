#!/usr/bin/expect

set timeout 10

set local_dir "/home/feng/project/video_cdn"
set guest_ip "192.168.56.101"
set password "123"

spawn scp -r $local_dir proj3@$guest_ip:~/bitrate-project-starter/
expect "password:"
send "$password\r"
interact

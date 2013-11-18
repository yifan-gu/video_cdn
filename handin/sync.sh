#!/usr/bin/expect

set timeout 10

set local_dir "/home/feng/project/video_cdn"
set guest_ip "172.16.64.129"
set password "123"

spawn scp -r $local_dir proj3@$guest_ip:~/bitrate-project-starter/
expect "password:"
send "$password\r"
interact

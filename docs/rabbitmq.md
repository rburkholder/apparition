# rabbitmq privilege commands for mqtt

* rabbitmqctl  add_user 'user' 'pass'
* rabbitmqctl set_user_tags user management
* rabbitmqctl set_permissions -p / user ".*" ".*" ".*"

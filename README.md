# Apparition - Home/Office Automation/Monitoring

_an immaterial appearance that seems real, and is generally sudden or startling in its manifestation -- something akin to what a monitoring and automation solution may appear to be_

The philosophy with Apparition is to compose an automation/monitoring/alert tool from a series of mature packages, all integrated under one roof.  The tool should work in the background and only manifest itself when needed.

Proposed tooling:
* C++ as primary backend
  * [boost](https://www.boost.org/) - use my [libs-build](https://github.com/rburkholder/libs-build) library to bulid and install
    * [asio](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio.html) - signals, network
    * [beast](https://www.boost.org/doc/libs/1_82_0/libs/beast/doc/html/index.html) - REST, WebSocket
  * [jwt-cpp](https://thalhammer.github.io/jwt-cpp/), [cpp-jwt](https://github.com/arun11299/cpp-jwt) - consideration for javascript websocket tokens
  * [wt](https://www.webtoolkit.eu/wt) - server side REST/UI
  * [yaml](https://github.com/jbeder/yaml-cpp) - YAML in/out for config files
    * [installation notes](docs/yaml-cpp.md)
    * [tutorial](https://github.com/jbeder/yaml-cpp/wiki/Tutorial)
    * [blog entry](https://www.fatalerrors.org/a/c-read-and-write-yaml-configuration-file.html)
  * Candidate scripting embedded scripting language:
    * [Duktape](https://duktape.org/guide.html) - embeddable ECMAScript® engine with a focus on portability and compact footprint
    * [LuaJIT](https://luajit.org/) - Just-In-Time Compiler for Lua
* [Bootstrap](https://getbootstrap.com/) - web page dynamics
* [Node-RED](https://nodered.org/) - visual event editor, used in addition to the embedded scripting language
* [Prometheus](https://prometheus.io/docs/introduction/overview/) - time series open-source systems monitoring and alerting toolkit
* [Grafana](https://grafana.com/) - data visualization aka visibility stack
  * [github](https://github.com/grafana/grafana)
* [RabbitMQ / MQTT](https://www.rabbitmq.com/mqtt.html) - event backbone

Thoughts influencing the design & implementation of this solution:

* config is supplied through a series of yaml files, which are primarily used to load scripts and attach the scripts to mqtt events
  * system configuration is thus easily version controlled, replaceable, and reproducible
* Prometheus supplies time series collection & management for historical review.  It offers compaction, particularly needed for sensors, such as the Ecowitt WS90 which issues multiple variables every 8 seconds or so.
* scripting language provides a rich set of interactive text based scripting capabilities for handling events, triggers, and logic
* MQTT broker to act as a backbone for messaging between sub-systems
* some draw backs of other solutions:
  * Home Assistant has a rather rigidily enforced installation if you want the base plus the store.  Plus there are comments that breakage occurs frequently during release cylces.
  * Domoticz is totally self contained and fast.  But it is missing certain signals such as the operational state of a thermstat.  The charts, based upon my limited experience, are not very informative.

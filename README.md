# Apparition - Home/Office Automation/Monitoring 

Apparition is an immaterial appearance that seems real, and is generally sudden or startling in its manifestation -- something akin to what a monitoring and automation solution may appear to be.

Proposed tooling:
* C++ as primary backbone
 * [daScript](https://dascript.org/) - automation scripting
   * [github](https://github.com/GaijinEntertainment/daScript)
   * [tutorial](https://github.com/GaijinEntertainment/daScript/blob/master/examples/tutorial/tutorial01.cpp)
   * [documentation](https://dascript.org/doc/index.html)
     * [index](https://dascript.org/doc/genindex.html)
 * https://github.com/jbeder/yaml-cpp - YAML in/out
   * [tutorial](https://github.com/jbeder/yaml-cpp/wiki/Tutorial)
   * [blog entry](https://www.fatalerrors.org/a/c-read-and-write-yaml-configuration-file.html)
  * [wt](https://www.webtoolkit.eu/wt) - server side REST/UI
* [Prometheus](https://prometheus.io/docs/introduction/overview/) - time series open-source systems monitoring and alerting toolkit
* [Grafana](https://grafana.com/) - data visualization aka visibility stack
  * [github](https://github.com/grafana/grafana)
* [Bootstrap](https://getbootstrap.com/) - web dynamics
* [RabbitMQ / MQTT](https://www.rabbitmq.com/mqtt.html) - event backbone
* [Node_Red](https://nodered.org/) - visual event editor


The philosophy with Apparition is to compose an automation/monitoring/alert tool from a series of mature packages integrated under one roof.

* Promeheus supplies time series collection & management for historical review.  It offers compaction, particularly needed for sensors, such as the Ecowitt WS90 which issues multiple variables every 8 seconds or so.
* daScript provides a rich text based scripting environment for handling events, triggers, and logic
* MQTT broker to act as a backbone for messaging between sub-systems
* some draw backs of other solutions:
  * Home Assistant has a rather rigidily enforced installation if you want the base plus the store.  Plus there are comments that breakage occurs frequently during release cylces.
  * Domoticz is totally self contained and fast.  But it is missing certain signals such as the operational state of a thermstat.  The charts, base upon my limited experience, are not very informative.

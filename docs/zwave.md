# zwave components

* zwave js ui: 
  * https://zwave-js.github.io/zwave-js-ui/#/getting-started/other-methods

```
cd ~
mkdir zwave-js-ui
cd zwave-js-ui
# download latest version
curl -s https://api.github.com/repos/zwave-js/zwave-js-ui/releases/latest  \
| grep "browser_download_url.*zip" \
| cut -d : -f 2,3 \
| tr -d \" \
| wget -i -
unzip zwave-js-ui-v*.zip
./zwave-js-ui
```

* controller to work with the above:
  * Zooz 700 Series Z-Wave Plus S2 USB Stick ZST10 700
    * https://www.amazon.ca/Z-Wave-Stick-Assistant-HomeSeer-Software/dp/B07GNZ56BK



'use strict';

/*
 * The websocket address for communication with the server.
 */
const WebSocketAddress = 'ws://' + document.location.host + '/';

/*
 * The DOM element that represents the symbol
 */
const Symbol = document.getElementById('#exchange-text > input:nth-child(1)');

const WebSocketMessageUpdate = {
  'type': 'update',
  'what': null,
  'exch': null,
  'secu': null
}

/*
 * Server/Client Communication
 */
let comms = class Socket
{

  /*
   * Creates a new connection to the server for communications
   * @param {string} address asdf
   */
  constructor(address, connectionEstablished)
  {
    this.connectionEstablished = connectionEstablished;
    this.address = address;
    this.ws = new WebSocket(address, 'lws-minimal');
    this.ws.onmessage = this.onmessage.bind(this);
    this.ws.onopen = this.onopen.bind(this);
  }

  /*
   * Sends a header update
   *
   * @return {None}
   */
  SendHeaderUpdate()
  {
    let wsmu = Object.assign({}, WebSocketMessageUpdate);
    wsmu.what = 'header';
    this.ws.send(JSON.stringify(wsmu));

    return;
  }

  SendSymbolUpdate(symbol, callback)
  {
    let wssu = Object.assign({}, WebSocketMessageUpdate);
    wssu.what = 'chart-full';
    wssu.secu = symbol;
    this.SendSymbolUpdateCallback = callback;
    this.ws.send(JSON.stringify(wssu));

    return;
  }

  SendSymbolUpdatePartial(symbol, callback)
  {
    let wssu = Object.assign({}, WebSocketMessageUpdate);
    wssu.what = 'chart-partial';
    wssu.secu = symbol;
    this.SendSymbolUpdateCallback = callback;
    this.ws.send(JSON.stringify(wssu));

    return;
  }

  /*
   * Handle what happens on the open of websocket. Open only happens when the
   * connection has been established.
   *
   * @param {Event} evt
   * @return {null}
   */
  onopen(evt)
  {
    console.log(evt);
    console.log('Connected to ' + this.address);
    this.connectionEstablished();
  }

  /*
   * Specifies the action upon the receving of a message from the server.
   * @param {Event} evt
   * @return {null}
   */
  onmessage(evt)
  {
    let data = JSON.parse(evt.data);

    if (!data.type)
    {
      console.error('error in message loop aborting all loops');
      return;
    }

    if (data.type == 'update')
    {
      if (data.what == 'header')
      {
        let headerList = data.data;
        let doc;
        for (let iter = 0; iter < headerList.length; ++iter)
        {
          doc = document.getElementById('header-bar-' + headerList[iter].symbol);
          doc.children[1].innerText =
            '$' +
            parseFloat((headerList[iter].bid/Math.pow(10,headerList[iter].precision)), headerList[iter].precision).toFixed(headerList[iter].precision);
        }
        setTimeout(this.SendHeaderUpdate.bind(this), 1000);
        return;
      }
      else if (data.what == 'chart-full')
      {
        this.SendSymbolUpdateCallback(data);
        return;
      }
      else if (data.what =='chart-partial')
      {
        this.SendSymbolUpdateCallback(data);
        return;
      }
    }
  }
}

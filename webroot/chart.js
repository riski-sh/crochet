'use strict';

const DrawingArea = document.getElementById('chart-drawable');
const DrawingContainer = document.getElementById('chart');

let chart = class Chart
{
  constructor(symbol)
  {
    this.symbol = symbol;

    this.context = DrawingArea.getContext('2d');
    this.candles = [];
    this.chartMax = Number.MIN_VALUE;
    this.chartMin = Number.MAX_VALUE;
    this.numericalPrecision = 0;
    this.bestBid = 0;
    this.bestAsk = 0;

    this.isMouseDown = false;
    this.mouseClickPosX = 0;
    this.mouseClickPosY = 0;

    this.mouseCurrentX = 0;
    this.mouseCurrentY = 0;

    this.shift = 0;
    this.tempShift = 0;

    this.sent = 1;
    this.received = 0;

    this.PricePixelTransformation = {'slope' : 0, 'inter' : 0};
    DrawingArea.onmousedown = this.onmousedown.bind(this);
    DrawingArea.onmouseup = this.onmouseup.bind(this);
    DrawingArea.onmousemove = this.onmousemove.bind(this);

    CommsChart.SendSymbolUpdate(symbol, this.loop.bind(this));
  }

  onmousedown(evt)
  {
    this.isMouseDown = true;
    this.mouseClickPosX = evt.pageX - DrawingArea.getBoundingClientRect().left;
    this.mouseClickPosY = evt.pageY - DrawingArea.getBoundingClientRect().top;
  }

  onmouseup(evt)
  {
    this.isMouseDown = false;
    this.shift = this.tempShift;
  }

  onmousemove(evt)
  {
    this.mouseCurrentX = evt.pageX - DrawingArea.getBoundingClientRect().left;
    this.mouseCurrentY = evt.pageY - DrawingArea.getBoundingClientRect().top;
  }

  getAxisWidth()
  {
    this.context.font = '14px Monospace';
    let metrics = Math.ceil(this.context.measureText(' 0.00000 ').width);
    return metrics;
  }

  getCharWidth()
  {
    let metrics = Math.ceil(this.context.measureText(' ').width);
    return metrics;
  }

  getTextHeight()
  {
    let metrics = this.context.measureText('0');
    return metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent;
  }

  setChartMinMax(offset)
  {
    this.chartMax = Number.MIN_VALUE;
    this.chartMin = Number.MAX_VALUE;
    for (let i = this.candles.length - offset - this.tempShift; i < this.candles.length - this.tempShift; ++i)
    {
      if (i >= 0) {
        if (this.candles[i].volume != 0)
        {
          if (this.chartMax < this.candles[i].high)
          {
            this.chartMax = this.candles[i].high;
          }

          if (this.chartMin > this.candles[i].low)
          {
            this.chartMin = this.candles[i].low;
          }
        }
      }
    }
    if (this.bestAsk > this.chartMax)
    {
      this.chartMax = this.bestAsk;
    }
  }

  setChartTransformations(offset)
  {
    this.setChartMinMax(offset);

    this.PricePixelTransformation.slope =
        (this.getTextHeight() - (DrawingArea.height - this.getTextHeight())) /
        (this.chartMax - this.chartMin);
    this.PricePixelTransformation.inter =
        (this.getTextHeight() -
         (this.PricePixelTransformation.slope * this.chartMax));
  }

  evalEquation(equation, zed)
  {
    return Math.floor((zed * equation.slope) + equation.inter);
  }

  loop(data)
  {
    this.received += 1;

    if (data.what == 'chart-full')
    {
      for (let i = 0; i < data.data.length; ++i)
      {
        this.candles[data.data[i].index] = data.data[i];
      }
      this.numericalPrecision = data.precision;
    }
    else if (data.what == 'chart-partial')
    {
      this.candles[data.data[0].index] = data.data[0];
      this.bestBid = data.bid;
      this.bestAsk = data.ask;
    }
    else
    {
      console.log('this is not my data sad :(');
    }

    let candleOccupationWidth = 8
    let candleRealWidth = 5;

    if (this.isMouseDown)
    {
      this.tempShift = this.shift + Math.floor((this.mouseCurrentX - this.mouseClickPosX) / 8);

      if (this.tempShift < 0)
      {
        this.tempShift = 0;
      }
      if (this.tempShift > this.candles.length)
      {
        this.tempShift = this.candles.length;
      }
    }

    let numCandles = Math.floor((DrawingArea.width - this.getAxisWidth()) /
                                candleOccupationWidth);

    if (DrawingContainer.getBoundingClientRect().width != DrawingArea.width ||
        DrawingContainer.getBoundingClientRect().height != DrawingArea.height)
    {
      DrawingArea.width = DrawingContainer.getBoundingClientRect().width;
      DrawingArea.height = DrawingContainer.getBoundingClientRect().height;
    }

    /* recalucate the transformations */
    this.setChartTransformations(numCandles);

    /* clear the drawing area */
    this.context.clearRect(0, 0, DrawingArea.width, DrawingArea.height);

    /* draw a bar to represent where the price axis on the right of the chart */
    this.context.strokeStyle = '#586e75';
    this.context.fillStyle = '#586e75';
    this.context.textBaseline = 'middle';

    this.context.setLineDash([]);
    this.context.lineWidth = '1px';
    this.context.moveTo(DrawingArea.width - this.getAxisWidth() + 0.5, 0);
    this.context.lineTo(DrawingArea.width - this.getAxisWidth() + 0.5,
                        DrawingArea.height);
    this.context.stroke();

    let seperator = Math.ceil((this.getTextHeight() + 1) /
                              -this.PricePixelTransformation.slope);
    if (seperator <= 1)
    {
      seperator = 2;
    }

    for (let x = this.chartMin; x <= this.chartMax; x += seperator)
    {
      let drawHeight = this.evalEquation(this.PricePixelTransformation, x);

      this.context.fillStyle = '#586e75';
      this.context.fillText(
          ' ' + parseFloat(x / (Math.pow(10, this.numericalPrecision)))
                    .toFixed(this.numericalPrecision),
          DrawingArea.width - this.getAxisWidth(), drawHeight + 1.5);

      this.context.strokeStyle = '#eee8d5';
      this.context.beginPath();
      this.context.moveTo(0, drawHeight + 0.5);
      this.context.lineTo(DrawingArea.width - this.getAxisWidth(),
                          drawHeight + 0.5);
      this.context.stroke();
    }
    this.context.setLineDash([]);

    let bar = DrawingArea.width - this.getAxisWidth();
    let bidHeight =
        this.evalEquation(this.PricePixelTransformation, this.bestBid);
    this.context.beginPath();
    this.context.moveTo(bar, bidHeight + 0.5);
    this.context.lineTo(bar + (this.getCharWidth()),
                        bidHeight + (this.getTextHeight() / 2) + 1);
    this.context.lineTo(DrawingArea.width,
                        bidHeight + (this.getTextHeight() / 2) + 1);
    this.context.lineTo(DrawingArea.width,
                        bidHeight - (this.getTextHeight() / 2) - 1);
    this.context.lineTo(bar + (this.getCharWidth()),
                        bidHeight - (this.getTextHeight() / 2) - 1);
    this.context.fillStyle = '#859900';
    this.context.fill();

    let askHeight =
        this.evalEquation(this.PricePixelTransformation, this.bestAsk);
    this.context.beginPath();
    this.context.moveTo(bar, askHeight + 0.5);
    this.context.lineTo(bar + (this.getCharWidth()),
                        askHeight + (this.getTextHeight() / 2) + 1);
    this.context.lineTo(DrawingArea.width,
                        askHeight + (this.getTextHeight() / 2) + 1);
    this.context.lineTo(DrawingArea.width,
                        askHeight - (this.getTextHeight() / 2) - 1);
    this.context.lineTo(bar + (this.getCharWidth()),
                        askHeight - (this.getTextHeight() / 2) - 1);

    this.context.fillStyle = '#dc322f';
    this.context.fill();

    this.context.fillStyle = '#fdf6e3';
    this.context.fillText(
        ' ' + parseFloat(this.bestBid / (Math.pow(10, this.numericalPrecision)))
                  .toFixed(this.numericalPrecision),
        DrawingArea.width - this.getAxisWidth(), bidHeight + 1.25);

    this.context.fillText(
        ' ' + parseFloat(this.bestAsk / (Math.pow(10, this.numericalPrecision)))
                  .toFixed(this.numericalPrecision),
        DrawingArea.width - this.getAxisWidth(), askHeight + 1.25);

    let startIndex = this.candles.length - numCandles - this.tempShift;
    if (startIndex < 0)
    {
      startIndex = 0;
    }

    for (let idx = startIndex; idx < this.candles.length - this.tempShift; ++idx)
    {

      let candle = this.candles[idx];

      if (candle.index % 20 == 0)
      {
        this.context.strokeStyle = '#93a1a1';
        this.context.beginPath();
        this.context.moveTo(((idx - startIndex + 1) * candleOccupationWidth) -
                                ((candleOccupationWidth - candleRealWidth) / 2),
                            0);
        this.context.lineTo(((idx - startIndex + 1) * candleOccupationWidth) -
                                ((candleOccupationWidth - candleRealWidth) / 2),
                            DrawingArea.height);
        this.context.stroke();
      }

      this.context.strokeStyle = '#073642';
      if (candle.volume == 0)
      {
        continue;
      }

      let open = this.evalEquation(this.PricePixelTransformation, candle.open);
      let high = this.evalEquation(this.PricePixelTransformation, candle.high);
      let low = this.evalEquation(this.PricePixelTransformation, candle.low);
      let close =
          this.evalEquation(this.PricePixelTransformation, candle.close);

      if (open > close)
      {
        this.context.beginPath();
        this.context.moveTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            high);
        this.context.lineTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            close);
        this.context.fillStyle = '#859900';
        this.context.fillRect(((idx - startIndex) * candleOccupationWidth),
                              close, candleRealWidth, open - close);
        this.context.moveTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            open);
        this.context.lineTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            low);
        this.context.stroke();
      }
      else if (open < close)
      {
        this.context.beginPath();
        this.context.moveTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            high);
        this.context.lineTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            open);
        this.context.fillStyle = '#dc322f';
        this.context.fillRect(((idx - startIndex) * candleOccupationWidth),
                              open, candleRealWidth, close - open);
        this.context.fillStyle = '#1A1A1A';
        this.context.moveTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            close);
        this.context.lineTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            low);
        this.context.stroke();
      }
      else
      {
        this.context.beginPath();
        this.context.moveTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            high);
        this.context.lineTo(((idx - startIndex) * candleOccupationWidth) +
                                Math.floor((candleRealWidth / 2.0)) + 0.5,
                            low);
        this.context.moveTo(((idx - startIndex) * candleOccupationWidth) - 0.5,
                            open + 0.5);
        this.context.lineTo(((idx - startIndex) * candleOccupationWidth) +
                                candleRealWidth,
                            open + 0.5);
        this.context.stroke();
      }
      this.context.stroke();

      let analysis = candle.analysis;
      for (let i = 0; i < analysis.length; ++i)
      {
        if (analysis[i].type == 'CHART_OBJECT_LINE')
        {
          this.context.strokeStyle = 'black';
          this.context.beginPath();
          this.context.moveTo(
              ((analysis[i].end - startIndex) * candleOccupationWidth) +
                  Math.floor((candleRealWidth / 2.0)) + 0.5,
              Math.floor(analysis[i].endPrice *
                             (this.PricePixelTransformation.slope) +
                         this.PricePixelTransformation.inter));
          this.context.lineTo(
              ((analysis[i].start - startIndex) * candleOccupationWidth) +
                  Math.floor((candleRealWidth / 2.0)) + 0.5,
              Math.floor(analysis[i].startPrice *
                             (this.PricePixelTransformation.slope) +
                         this.PricePixelTransformation.inter));
          this.context.stroke();
        }
      }
    }

    if (this.sent == this.received)
    {
      this.sent += 1;
      window.setTimeout(() => {
        CommsChart.SendSymbolUpdatePartial(this.symbol, this.loop.bind(this));
      }, 33);
    }
  }
}

const CommsChart =
    new comms(WebSocketAddress, () => { const Chart = new chart('EUR_USD'); });

const CommsHeader =
    new comms(WebSocketAddress, () => { CommsHeader.SendHeaderUpdate(); });

R"html(
<html>
    <head>
        <title>Label Designer</title>
        <link rel="stylesheet" href="https://cdn.simplecss.org/simple.min.css"/>
        <style>

            :root {
            --accent: #ffb300;
            --accent-hover: #ffe099;
            --accent-text: var(--bg);
            }

            body {
              background-color: #ded3f0;
                margin: 0;
                grid-template-columns: 1fr;
                grid-template-rows: 60px 1fr;
            }

            .container {
              background-color: #7755aa;
              color: #fff;
              padding: 20px 10px;
              grid-column: 1;
            }

            ul li strong {
                display: inline-block;
                width: 90px;
            }

            ul li.active {
                color: #ffff88;
            }

            div#main-controls {
                width: 500px;
                display: grid; 
                grid-template-columns: 1fr 1fr 1fr 1fr 1fr 1fr; 
                column-gap: 10px;
                grid-template-areas: 
                "letter letter letter-width letter-width grid-size grid-size"
                "code code code code code code"
                "font-select font-select font-select letter-select letter-select letter-select";
            }
            div#main-controls input,
            div#main-controls select,
            div#main-controls textarea {
                width: 100%;
                max-width: 100%;
            }

            div.scale-label {
                background-color: #333;
                color: #fff;
                padding: 5px 10px;
                font-size: 1em;
                border: #fff 1px solid;
                font-family: monospace;
            }
            div.scale-label span.define {
                color: #ff7799;
            }
            div.scale-label span.macro {
                color: #8888ff;
            }
            div.scale-label #scale-value {
                color: #88ff88;
            }

            #arduino-code {
                display: none;
            }
            .lity-container #arduino-code {
                display: block;
                margin: 4em;
                overflow-y: scroll;
            }
            button:disabled {
                color: #aaa;
                cursor: not-allowed !important;
            }
            button.small {
                font-size: 12px;
            }
            button.active {
                background-color: #cc66ff;
                border-color: #aa33bb;
                color: #666;
            }
            #btn-pen,
            #btn-print {
              padding: 13px 20px;
            }


            .modal-overlay {
              position: fixed;
              top: 0;
              left: 0;
              width: 100%;
              height: 100%;
              background-color: rgba(0, 0, 0, 0.5);
              display: none; /* Hidden by default */
              justify-content: center;
              align-items: center;
              z-index: 1000;
            }

            .modal {
              display: none;
              background-color: white;
              color: black;
              padding: 20px;
              border-radius: 8px;
              min-width: 300px;
              position: relative;
              box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
            }
            .modal-overlay .modal {
              display: block; /* Show the modal when the overlay is active */
            }

            .modal .close-button {
              position: absolute;
              top: 10px;
              right: 15px;
              font-size: 24px;
              cursor: pointer;
            }
            .modal h3 {
              margin: 0 0 0.5em 0;
            }
            .modal input,
            .modal select {
              width: 100%;
              max-width: 100%;
            }




    .tabs {
      display: flex;
      margin: 10px 10px 1px 10px;
      position: relative;
      grid-column: 1;
    }

    .tab {
      padding: 12px 24px;
      background-color: #a892c8;
      border: 1px solid #ccc;
      border-bottom: none;
      border-radius: 8px 8px 0 0;
      margin-right: 4px;
      z-index: 1;
      position: relative;
      top: 1px;
      color: #fff;
    }

    .tab.active {
      background-color: #7755aa;

      font-weight: bold;
      z-index: 2;
    }

    a.tab:hover {
      background-color: #907dae;
    }

    .tab-content {
      border: 1px solid #ccc;
      background: white;
      padding: 20px;
      border-radius: 0 8px 8px 8px;
      display: none;
    }

    .tab-content.active {
      display: block;
    }


    .page-container::-webkit-scrollbar {
  height: 12px;
}

.page-container::-webkit-scrollbar-thumb {
  background-color: rgba(0, 0, 0, 0.4);
  border-radius: 6px;
}

.page-container::-webkit-scrollbar-track {
  background: rgba(0, 0, 0, 0.1);
}

        </style>
    </head>
    <body>

      <div class="tabs">
        <span class="tab active">Label Designer</span>
        <a class="tab" href="/editor">Font Editor</a>
      </div>

        <div class="container">
            <div style="overflow-x: scroll; width: calc(100vw - 20px);border: solid 2px #fff" class="page-container">
                <canvas id="drawing" width="2500" height="300"></canvas>        
            </div>
            <div style="display: flex; gap: 10px; padding: 10px 0; justify-content: space-between;">
              <div>
              <button id="btn-select" class="tool active">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-cursor" viewBox="0 0 16 16">
                  <path d="M14.082 2.182a.5.5 0 0 1 .103.557L8.528 15.467a.5.5 0 0 1-.917-.007L5.57 10.694.803 8.652a.5.5 0 0 1-.006-.916l12.728-5.657a.5.5 0 0 1 .556.103zM2.25 8.184l3.897 1.67a.5.5 0 0 1 .262.263l1.67 3.897L12.743 3.52z"/>
                </svg>
              </button>
              <button id="btn-rect" class="tool">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-square" viewBox="0 0 16 16">
                  <path d="M14 1a1 1 0 0 1 1 1v12a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1zM2 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V2a2 2 0 0 0-2-2z"/>
                </svg>
              </button>
              <button id="btn-ellipse" class="tool">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-circle" viewBox="0 0 16 16">
                  <path d="M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14m0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16"/>
                </svg>
              </button>
              <button id="btn-line" class="tool">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-diagonal-line" viewBox="0 0 16 16">
                  <line x1="0" y1="0" x2="16" y2="16" stroke="currentColor" stroke-width="2"/>
                </svg>                
              </button>
              <button id="btn-bez" class="tool">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-bezier2" viewBox="0 0 16 16">
                  <path fill-rule="evenodd" d="M1 2.5A1.5 1.5 0 0 1 2.5 1h1A1.5 1.5 0 0 1 5 2.5h4.134a1 1 0 1 1 0 1h-2.01q.269.27.484.605C8.246 5.097 8.5 6.459 8.5 8c0 1.993.257 3.092.713 3.7.356.476.895.721 1.787.784A1.5 1.5 0 0 1 12.5 11h1a1.5 1.5 0 0 1 1.5 1.5v1a1.5 1.5 0 0 1-1.5 1.5h-1a1.5 1.5 0 0 1-1.5-1.5H6.866a1 1 0 1 1 0-1h1.711a3 3 0 0 1-.165-.2C7.743 11.407 7.5 10.007 7.5 8c0-1.46-.246-2.597-.733-3.355-.39-.605-.952-1-1.767-1.112A1.5 1.5 0 0 1 3.5 5h-1A1.5 1.5 0 0 1 1 3.5zM2.5 2a.5.5 0 0 0-.5.5v1a.5.5 0 0 0 .5.5h1a.5.5 0 0 0 .5-.5v-1a.5.5 0 0 0-.5-.5zm10 10a.5.5 0 0 0-.5.5v1a.5.5 0 0 0 .5.5h1a.5.5 0 0 0 .5-.5v-1a.5.5 0 0 0-.5-.5z"/>
                </svg>
              </button>
              <button id="btn-text" class="tool">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-fonts" viewBox="0 0 16 16">
                  <path d="M12.258 3h-8.51l-.083 2.46h.479c.26-1.544.758-1.783 2.693-1.845l.424-.013v7.827c0 .663-.144.82-1.3.923v.52h4.082v-.52c-1.162-.103-1.306-.26-1.306-.923V3.602l.431.013c1.934.062 2.434.301 2.693 1.846h.479z"/>
                </svg>
              </button>
              <button id="btn-delete" class="">
                <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" class="bi bi-trash3" viewBox="0 0 16 16">
                  <path d="M6.5 1h3a.5.5 0 0 1 .5.5v1H6v-1a.5.5 0 0 1 .5-.5M11 2.5v-1A1.5 1.5 0 0 0 9.5 0h-3A1.5 1.5 0 0 0 5 1.5v1H1.5a.5.5 0 0 0 0 1h.538l.853 10.66A2 2 0 0 0 4.885 16h6.23a2 2 0 0 0 1.994-1.84l.853-10.66h.538a.5.5 0 0 0 0-1zm1.958 1-.846 10.58a1 1 0 0 1-.997.92h-6.23a1 1 0 0 1-.997-.92L3.042 3.5zm-7.487 1a.5.5 0 0 1 .528.47l.5 8.5a.5.5 0 0 1-.998.06L5 5.03a.5.5 0 0 1 .47-.53Zm5.058 0a.5.5 0 0 1 .47.53l-.5 8.5a.5.5 0 1 1-.998-.06l.5-8.5a.5.5 0 0 1 .528-.47M8 4.5a.5.5 0 0 1 .5.5v8.5a.5.5 0 0 1-1 0V5a.5.5 0 0 1 .5-.5"/>
                </svg>
              </button>
            </div>
            <div>
              <button id="btn-pen" class="">Insert Pen</button>
            </div>
            <div>
              <button id="btn-print" class="">Print</button>
            </div>
          </div>
        </div>

        <div id="modals">

          <div class="modal" id="add-text-modal">
            <span class="close-button">&times;</span>
            <h3>Add Text</h3>
            <label>Font</label>
            <select id="font"></select><br />
            <label>Font Size</label>
            <select id="font-size">
              <option value="12">12</option>
              <option value="16">16</option>
              <option value="20">20</option>
              <option value="24">24</option>
              <option value="28">28</option>
              <option value="32">32</option>
              <option value="36">36</option>
              <option value="40">40</option>
              <option value="44">44</option>
              <option value="48">48</option>
              <option value="52">52</option>
              <option value="56">56</option>
            </select>
            <br />
            <label>Text</label>
            <input type="text" id="text" value=""/><br />
            <button id="btn-add-text" class="button">Add</button>
          </div>

          <div class="modal" id="insert-pen-modal">
            <h3>Insert Pen</h3>
            <p>Insert the pen into the label maker ensuring the tip is firmly against the tape.</p>
            <button id="btn-lift-pen" class="button">Done</button>
          </div>

        </div>

        <div id="modalOverlay" class="modal-overlay"></div>

    </body>
    <script src="label-designer.js"></script>
</html>

)html"
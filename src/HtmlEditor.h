R"html(
<html>
    <head>
        <title>Letter Editor</title>
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
            .modal input {
              width: 100%;
              max-width: 100%;
            }



        </style>
    </head>
    <body>

      <div class="tabs">
        <a class="tab" href="/">Label Designer</a>
        <span class="tab active">Font Editor</span>
      </div>

        <div class="container">


        <div style="display: flex; gap: 10px">
            <div>
                <canvas id="drawing" width="500" height="500"></canvas>
                <div>
                    <ul>
                        <li id="s" class="tool active"><strong>S/Space:</strong> Select Objects Tool</li>
                        <li id="l" class="tool"><strong>L:</strong> Draw Line Tool</li>
                        <li id="b" class="tool"><strong>B:</strong> Draw Bezier Tool</li>
                        <li><strong>D/DEL:</strong> Delete Selected Objects</li>
                        <li><strong>Arrows:</strong> Move Selected Objects</li>
                        
                    </ul>
        
                </div>
        
            </div>
            <div>
                <div id="main-controls">
                    <div style="grid-area: letter;">
                        <span>Letter</span><br />
                        <input type="text" id="letter" value="">
                    </div>
                    <div style="grid-area: letter-width;">
                        <span>Letter Width</span><br />
                        <select id="letterwidth">
                            <option value="1">1</option>
                            <option value="2">2</option>
                            <option value="3">3</option>
                            <option value="4">4</option>
                            <option value="5">5</option>
                            <option value="6">6</option>
                            <option value="7">7</option>
                            <option value="8">8</option>
                            <option value="9">9</option>
                            <option value="10">10</option>
                            <option value="11">11</option>
                            <option value="12">12</option>
                            <option value="13" selected>13</option>
                            <option value="14">14</option>
                            <option value="15">15</option>
                            <option value="16">16</option>
                        </select>
                    </div>
                    <div style="grid-area: grid-size;">
                        <span>Grid Size</span><br />
                        <select id="gridsize">
                            <option value="5">5</option>
                            <option value="6">6</option>
                            <option value="7">7</option>
                            <option value="8">8</option>
                            <option value="9">9</option>
                            <option value="10">10</option>
                            <option value="11">11</option>
                            <option value="12">12</option>
                            <option value="13" selected>13</option>
                            <option value="14">14</option>
                            <option value="15">15</option>
                            <option value="16">16</option>
                         </select>
                    </div>

                    <div style="grid-area: code; padding-bottom: 10px;">
                        <textarea id="code" rows="20" cols="50" style="height: 380px;"></textarea>
                        <div class="scale-label"><span class="define">#define</span> <span class="macro">SCALE_VALUE</span> <span id="scale-value">65</span></div>    
                    </div>

                    <div style="grid-area: font-select;">
                        <span>Font</span><br />
                        <select id="font-select" disabled>
                        </select>
                        <button id="create-font" class="button">Create</button>
                        <button id="delete-font" class="button" disabled>Delete</button><br />
                        <button id="import-font" class="button">Import</button>
                        <button id="export-font" class="button" disabled>Export</button>
                    </div>
                    <div style="grid-area: letter-select;">
                        <span>Letter</span><br />
                        <select id="letter-select" disabled>
                            <option value="0">[Create Font First]</option>
                        </select>
                        <button id="save-letter" class="button" disabled>Save</button>
                        <button id="delete-letter" class="button" disabled>Delete</button>
                    </div>
                </div>

            </div>

        </div>

        <div id="modals">

          <div class="modal" id="import-modal">
            <span class="close-button" id="closeModalBtn">&times;</span>
            <h3>Import Font</h3>
            <label>Font Name</label>
            <input type="text" id="import-font-name" value="" placeholder="Font Name"><br />
            <label>Font Data</label>
            <textarea id="import-font-data" rows="10" cols="50" placeholder="{'?', (const uint8_t[]){ 0x00..."></textarea><br />
            <button id="start-import-font" class="button">Import</button>
          </div>

          <div class="modal" id="export-modal">
            <span class="close-button" id="closeModalBtn">&times;</span>
            <h3>Export Font</h3>
            <label>Code</label>
            <textarea id="export-font-data" rows="10" cols="50"></textarea><br />
          </div>

        </div>

      </div>


        <div id="modalOverlay" class="modal-overlay"></div>

    </body>
    <script src="letter-editor.js"></script>
</html>

)html"
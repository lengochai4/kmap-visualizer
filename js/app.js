/**
 * Main Web App Controller
 * Connects KMapEngine, CircuitRenderer and UI elements
 * Group 8 - Discrete Mathematics & Graph Theory (HCMUTE)
 */

document.addEventListener('DOMContentLoaded', () => {
    const engine = new window.KMapEngine();
    const circuitSvg = document.getElementById('circuit-svg');
    const circuitRenderer = new window.CircuitRenderer(circuitSvg);

    // DOM Elements
    const exprInput = document.getElementById('expr-input');
    const btnSolve = document.getElementById('btn-solve');
    const btnClear = document.getElementById('btn-clear');
    const numVarsSelect = document.getElementById('num-vars-select');
    const virtualKeys = document.querySelectorAll('.btn-key');
    const sampleChips = document.querySelectorAll('.sample-chip');
    const casesContainer = document.getElementById('cases-container');
    const equationDisplay = document.getElementById('equation-display');
    const piContainer = document.getElementById('pi-container');
    const epiContainer = document.getElementById('epi-container');
    const kmapContainer = document.getElementById('kmap-container');
    const kmapTabs = document.querySelectorAll('.kmap-tab');
    const btnCredits = document.getElementById('btn-credits');
    const modalCredits = document.getElementById('modal-credits');
    const modalClose = document.getElementById('modal-close');

    // State
    let currentVariables = ['A', 'B', 'C'];
    let currentGrid = [];
    let currentPIs = [];
    let currentSolutions = [];
    let selectedCaseIndex = 0;
    let activeTab = 'minimized'; // 'initial' or 'minimized'

    // Color palette for K-Map groups
    const groupColors = [
        { fill: 'rgba(236, 72, 153, 0.22)', stroke: '#ec4899' },
        { fill: 'rgba(56, 189, 248, 0.22)', stroke: '#38bdf8' },
        { fill: 'rgba(16, 185, 129, 0.22)', stroke: '#10b981' },
        { fill: 'rgba(245, 158, 11, 0.22)', stroke: '#f59e0b' },
        { fill: 'rgba(168, 85, 247, 0.22)', stroke: '#a855f7' },
        { fill: 'rgba(239, 68, 68, 0.22)', stroke: '#ef4444' }
    ];

    /**
     * Solves the given Boolean expression and updates all views
     */
    function solveExpression() {
        const rawExpr = exprInput.value.trim();
        let vars = engine.extractVariables(rawExpr);

        // Synchronize with variable dropdown if set
        const selectedVarCount = parseInt(numVarsSelect.value, 10);
        if (selectedVarCount && vars.length !== selectedVarCount) {
            // Adjust variable list length
            const standardVars = ['A', 'B', 'C', 'D'];
            vars = standardVars.slice(0, selectedVarCount);
        }

        currentVariables = vars;
        numVarsSelect.value = vars.length.toString();

        const minterms = engine.parseExpressionToMinterms(rawExpr, currentVariables);
        currentGrid = engine.buildGrid(minterms, currentVariables.length);

        computeAndRender();
    }

    /**
     * Runs K-Map minimization from the current grid state
     */
    function computeAndRender() {
        currentPIs = engine.findPrimeImplicants(currentGrid);
        currentSolutions = engine.solveMinimalCovers(currentGrid, currentPIs, currentVariables);
        selectedCaseIndex = 0;

        renderResults();
        renderKMap();
        renderCircuit();
    }

    /**
     * Renders minimal covers tabs, equation and PIs/EPIs
     */
    function renderResults() {
        // Render Case buttons
        casesContainer.innerHTML = '';
        currentSolutions.forEach((sol, idx) => {
            const btn = document.createElement('button');
            btn.className = `case-btn ${idx === selectedCaseIndex ? 'active' : ''}`;
            btn.textContent = `Case ${idx + 1}`;
            btn.addEventListener('click', () => {
                selectedCaseIndex = idx;
                document.querySelectorAll('.case-btn').forEach((b, i) => {
                    b.classList.toggle('active', i === idx);
                });
                renderActiveCase();
            });
            casesContainer.appendChild(btn);
        });

        // Prime Implicants list
        piContainer.innerHTML = '';
        currentPIs.forEach(pi => {
            const span = document.createElement('span');
            span.className = 'pi-tag';
            span.textContent = engine.groupToTerm(pi, currentVariables);
            piContainer.appendChild(span);
        });

        renderActiveCase();
    }

    function renderActiveCase() {
        if (!currentSolutions.length) {
            equationDisplay.textContent = 'F = 0';
            circuitRenderer.render('0', currentVariables);
            return;
        }

        const activeSol = currentSolutions[selectedCaseIndex] || currentSolutions[0];
        equationDisplay.textContent = `F = ${activeSol.expression}`;

        renderKMap();
        renderCircuit();
    }

    /**
     * Renders K-Map table grid and group enclosures
     */
    function renderKMap() {
        kmapContainer.innerHTML = '';
        const n = currentVariables.length;
        const split = Math.floor(n / 2);
        const rowVars = currentVariables.slice(0, split).join('');
        const colVars = currentVariables.slice(split).join('');

        const rowCount = 1 << split;
        const colCount = 1 << (n - split);

        const tableWrapper = document.createElement('div');
        tableWrapper.className = 'kmap-table-wrapper';

        const table = document.createElement('table');
        table.className = 'kmap-table';

        // Table Header
        const thead = document.createElement('thead');
        const headerRow = document.createElement('tr');

        // Corner cell
        const cornerCell = document.createElement('th');
        cornerCell.className = 'kmap-corner';
        cornerCell.textContent = `${rowVars} \\ ${colVars}`;
        headerRow.appendChild(cornerCell);

        // Column headers (Gray code)
        for (let c = 0; c < colCount; c++) {
            const th = document.createElement('th');
            th.className = 'kmap-header-col';
            th.textContent = engine.indexToGray(c, n - split);
            headerRow.appendChild(th);
        }
        thead.appendChild(headerRow);
        table.appendChild(thead);

        // Table Body
        const tbody = document.createElement('tbody');
        for (let r = 0; r < rowCount; r++) {
            const tr = document.createElement('tr');

            // Row header
            const th = document.createElement('th');
            th.className = 'kmap-header-row';
            th.textContent = engine.indexToGray(r, split);
            tr.appendChild(th);

            // Cells
            for (let c = 0; c < colCount; c++) {
                const td = document.createElement('td');
                const val = currentGrid[r] ? currentGrid[r][c] : 0;
                td.className = `kmap-cell val-${val}`;
                td.textContent = val;
                td.dataset.row = r;
                td.dataset.col = c;

                // Interactive click: flip 0 <-> 1
                td.addEventListener('click', () => {
                    currentGrid[r][c] = currentGrid[r][c] === 1 ? 0 : 1;
                    td.className = `kmap-cell val-${currentGrid[r][c]}`;
                    td.textContent = currentGrid[r][c];

                    // Reconstruct expression from grid
                    updateExprFromGrid();
                    computeAndRender();
                });

                tr.appendChild(td);
            }
            tbody.appendChild(tr);
        }
        table.appendChild(tbody);
        tableWrapper.appendChild(table);

        // Render SVG overlay for groups if in 'minimized' tab
        if (activeTab === 'minimized' && currentSolutions.length) {
            const overlaySvg = createKMapGroupOverlay(tableWrapper, rowCount, colCount);
            tableWrapper.appendChild(overlaySvg);
        }

        kmapContainer.appendChild(tableWrapper);
    }

    /**
     * Draws SVG rounded group loops over the K-Map cells, with wrap-around support
     */
    function createKMapGroupOverlay(wrapper, rowCount, colCount) {
        const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        svg.setAttribute('class', 'kmap-overlay-svg');

        // Let table render first to calculate actual cell coordinates
        setTimeout(() => {
            const table = wrapper.querySelector('table');
            if (!table) return;

            const tableRect = table.getBoundingClientRect();
            svg.setAttribute('width', tableRect.width);
            svg.setAttribute('height', tableRect.height);
            svg.setAttribute('viewBox', `0 0 ${tableRect.width} ${tableRect.height}`);

            const activeSol = currentSolutions[selectedCaseIndex] || currentSolutions[0];
            const groups = activeSol ? activeSol.groups : [];

            groups.forEach((g, gIdx) => {
                const color = groupColors[gIdx % groupColors.length];

                // Split group into intervals for cylindrical/torus wrap-around
                const rowIntervals = [];
                if (g.sRow + g.rSize <= rowCount) {
                    rowIntervals.push({ start: g.sRow, size: g.rSize });
                } else {
                    rowIntervals.push({ start: g.sRow, size: rowCount - g.sRow });
                    rowIntervals.push({ start: 0, size: g.sRow + g.rSize - rowCount });
                }

                const colIntervals = [];
                if (g.sCol + g.cSize <= colCount) {
                    colIntervals.push({ start: g.sCol, size: g.cSize });
                } else {
                    colIntervals.push({ start: g.sCol, size: colCount - g.sCol });
                    colIntervals.push({ start: 0, size: g.sCol + g.cSize - colCount });
                }

                rowIntervals.forEach(rInt => {
                    colIntervals.forEach(cInt => {
                        const firstCell = table.querySelector(`.kmap-cell[data-row="${rInt.start}"][data-col="${cInt.start}"]`);
                        const lastCell = table.querySelector(`.kmap-cell[data-row="${rInt.start + rInt.size - 1}"][data-col="${cInt.start + cInt.size - 1}"]`);

                        if (firstCell && lastCell) {
                            const fRect = firstCell.getBoundingClientRect();
                            const lRect = lastCell.getBoundingClientRect();

                            const x = fRect.left - tableRect.left + 5;
                            const y = fRect.top - tableRect.top + 5;
                            const w = (lRect.right - fRect.left) - 10;
                            const h = (lRect.bottom - fRect.top) - 10;

                            const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                            rect.setAttribute('x', x);
                            rect.setAttribute('y', y);
                            rect.setAttribute('width', w);
                            rect.setAttribute('height', h);
                            rect.setAttribute('rx', '14');
                            rect.setAttribute('fill', color.fill);
                            rect.setAttribute('stroke', color.stroke);
                            rect.setAttribute('stroke-width', '2.5');
                            svg.appendChild(rect);
                        }
                    });
                });
            });
        }, 10);

        return svg;
    }

    /**
     * Generates a canonical SOP string from currentGrid
     */
    function updateExprFromGrid() {
        const n = currentVariables.length;
        const onesMinterms = [];
        for (let r = 0; r < currentGrid.length; r++) {
            for (let c = 0; c < currentGrid[0].length; c++) {
                if (currentGrid[r][c] === 1) {
                    const bits = engine.coordsToBits(r, c, n);
                    let term = '';
                    for (let k = 0; k < n; k++) {
                        term += (bits[k] === '1' ? currentVariables[k] : `-${currentVariables[k]}`) + ' ';
                    }
                    onesMinterms.push(term.trim());
                }
            }
        }
        exprInput.value = onesMinterms.length ? onesMinterms.join(' + ') : '0';
    }

    /**
     * Renders logic circuit diagram
     */
    function renderCircuit() {
        const activeSol = currentSolutions[selectedCaseIndex] || currentSolutions[0];
        const expr = activeSol ? activeSol.expression : '0';
        circuitRenderer.render(expr, currentVariables);
    }

    // =========================================================================
    // Event Listeners
    // =========================================================================

    // Solve Button
    btnSolve.addEventListener('click', solveExpression);

    // Enter Key in Input
    exprInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            solveExpression();
        }
    });

    // Clear Button
    btnClear.addEventListener('click', () => {
        exprInput.value = '';
        exprInput.focus();
    });

    // Variable count dropdown changed
    numVarsSelect.addEventListener('change', (e) => {
        const count = parseInt(e.target.value, 10);
        const standardVars = ['A', 'B', 'C', 'D'];
        currentVariables = standardVars.slice(0, count);

        // Reset grid for new variable size
        const rowCount = 1 << Math.floor(count / 2);
        const colCount = 1 << (count - Math.floor(count / 2));
        currentGrid = Array.from({ length: rowCount }, () => new Array(colCount).fill(0));

        // Sample expression
        if (count === 2) exprInput.value = 'A B + -A B';
        else if (count === 3) exprInput.value = 'A B + -A C';
        else exprInput.value = 'A B C D + -A -B -C -D + B -C D';

        solveExpression();
    });

    // Virtual Keyboard
    virtualKeys.forEach(btn => {
        btn.addEventListener('click', () => {
            const val = btn.dataset.val;
            if (val === 'backspace') {
                exprInput.value = exprInput.value.slice(0, -1);
            } else {
                exprInput.value += val;
            }
            exprInput.focus();
        });
    });

    // Sample Expression Chips
    sampleChips.forEach(chip => {
        chip.addEventListener('click', () => {
            exprInput.value = chip.dataset.expr;
            const varsCount = parseInt(chip.dataset.vars, 10);
            if (varsCount) {
                numVarsSelect.value = varsCount.toString();
            }
            solveExpression();
        });
    });

    // K-Map Tab Switcher
    kmapTabs.forEach(tab => {
        tab.addEventListener('click', () => {
            kmapTabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            activeTab = tab.dataset.tab;
            renderKMap();
        });
    });

    // Modal Credits
    btnCredits.addEventListener('click', () => modalCredits.classList.add('active'));
    modalClose.addEventListener('click', () => modalCredits.classList.remove('active'));
    modalCredits.addEventListener('click', (e) => {
        if (e.target === modalCredits) modalCredits.classList.remove('active');
    });

    // Initial Execution
    solveExpression();
});

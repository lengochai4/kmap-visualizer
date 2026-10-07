/**
 * Interactive SVG Logic Circuit Drawer
 * Draws IEEE Standard Logic Gates (AND, OR, NOT) with orthogonal wire routing
 * Group 8 - Discrete Mathematics & Graph Theory (HCMUTE)
 */

class CircuitRenderer {
    constructor(svgElement) {
        this.svg = svgElement;
        this.ns = 'http://www.w3.org/2000/svg';
    }

    clear() {
        while (this.svg.firstChild) {
            this.svg.removeChild(this.svg.firstChild);
        }
    }

    createElement(tag, attrs = {}) {
        const el = document.createElementNS(this.ns, tag);
        for (let [key, val] of Object.entries(attrs)) {
            el.setAttribute(key, val);
        }
        return el;
    }

    /**
     * Renders logic circuit diagram from a SOP Boolean expression
     * e.g. "A B + -A C + D" or "1" or "0"
     */
    render(expression, variables) {
        this.clear();
        const expr = (expression || '').trim();

        // Canvas settings
        const width = 850;
        const height = 450;
        this.svg.setAttribute('viewBox', `0 0 ${width} ${height}`);

        // Defs: shadows, filters, markers
        const defs = this.createElement('defs');
        defs.innerHTML = `
            <filter id="glow" x="-20%" y="-20%" width="140%" height="140%">
                <feDropShadow dx="0" dy="2" stdDeviation="3" flood-color="rgba(0,0,0,0.25)"/>
            </filter>
            <linearGradient id="gateGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#38bdf8"/>
                <stop offset="100%" stop-color="#0284c7"/>
            </linearGradient>
            <linearGradient id="orGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#f472b6"/>
                <stop offset="100%" stop-color="#db2777"/>
            </linearGradient>
            <linearGradient id="notGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#fbbf24"/>
                <stop offset="100%" stop-color="#d97706"/>
            </linearGradient>
        `;
        this.svg.appendChild(defs);

        // Constant cases: 0 or 1
        if (expr === '0' || expr === '1') {
            this.renderConstant(expr, width, height);
            return;
        }

        const rawTerms = expr.split('+').map(t => t.trim()).filter(Boolean);
        if (rawTerms.length === 0) {
            this.renderConstant('0', width, height);
            return;
        }

        // Layout Geometry
        const startX = 60;
        const busSpacing = 35;
        const inputYTop = 50;
        const inputYBottom = height - 50;
        const busXMap = {};

        // Draw vertical input buses for each variable
        variables.forEach((v, idx) => {
            const bx = startX + idx * busSpacing;
            busXMap[v] = bx;

            // Pin Header
            const pinCircle = this.createElement('circle', {
                cx: bx, cy: inputYTop - 15, r: 14,
                fill: '#6366f1', filter: 'url(#glow)'
            });
            const pinText = this.createElement('text', {
                x: bx, y: inputYTop - 10,
                'text-anchor': 'middle', 'font-size': '12',
                'font-weight': 'bold', fill: '#ffffff'
            });
            pinText.textContent = v;

            // Bus Line
            const busLine = this.createElement('line', {
                x1: bx, y1: inputYTop, x2: bx, y2: inputYBottom,
                stroke: 'rgba(99, 102, 241, 0.4)', 'stroke-width': '2'
            });

            this.svg.appendChild(busLine);
            this.svg.appendChild(pinCircle);
            this.svg.appendChild(pinText);
        });

        // Parse each product term into literals
        const termsData = rawTerms.map((termStr) => {
            const literals = [];
            const tokens = termStr.split(/\s+/).filter(Boolean);
            for (let tok of tokens) {
                let isNeg = tok.startsWith('-') || tok.startsWith('~') || tok.startsWith('!') || tok.endsWith("'");
                let cleanVar = tok.replace(/[-~!']/g, '').toUpperCase();
                if (variables.includes(cleanVar)) {
                    literals.push({ var: cleanVar, isNeg });
                }
            }
            return { termStr, literals };
        });

        const termCount = termsData.length;
        const availableHeight = height - 120;
        const termSpacing = availableHeight / Math.max(termCount, 1);
        const gatesX = startX + variables.length * busSpacing + 120;
        const termOutputs = [];

        // Render each product term (AND gate or single literal)
        termsData.forEach((term, tIdx) => {
            const gateY = inputYTop + 30 + tIdx * termSpacing;
            const literals = term.literals;

            if (literals.length > 1) {
                // Multi-literal: Draw AND Gate
                const andGate = this.drawANDGate(gatesX, gateY - 20, 45, 40);
                this.svg.appendChild(andGate);

                // Inputs to AND gate
                const inputSpacing = 36 / (literals.length + 1);
                literals.forEach((lit, lIdx) => {
                    const inY = (gateY - 20) + (lIdx + 1) * inputSpacing;
                    const bx = busXMap[lit.var];

                    if (lit.isNeg) {
                        // Draw NOT Gate before AND gate
                        const notX = gatesX - 45;
                        const notGate = this.drawNOTGate(notX, inY - 8, 22, 16);
                        this.svg.appendChild(notGate);

                        // Wire from Bus to NOT
                        this.drawWire(bx, inY, notX, inY);
                        // Wire from NOT to AND
                        this.drawWire(notX + 22, inY, gatesX, inY);
                    } else {
                        // Direct wire from Bus to AND
                        this.drawWire(bx, inY, gatesX, inY);
                    }

                    // Connection Dot on Bus
                    const dot = this.createElement('circle', {
                        cx: bx, cy: inY, r: 3.5, fill: '#6366f1'
                    });
                    this.svg.appendChild(dot);
                });

                termOutputs.push({ x: gatesX + 45, y: gateY });
            } else if (literals.length === 1) {
                // Single Literal term
                const lit = literals[0];
                const bx = busXMap[lit.var];

                if (lit.isNeg) {
                    const notX = gatesX - 20;
                    const notGate = this.drawNOTGate(notX, gateY - 8, 24, 16);
                    this.svg.appendChild(notGate);
                    this.drawWire(bx, gateY, notX, gateY);

                    const dot = this.createElement('circle', {
                        cx: bx, cy: gateY, r: 3.5, fill: '#6366f1'
                    });
                    this.svg.appendChild(dot);
                    termOutputs.push({ x: notX + 24, y: gateY });
                } else {
                    this.drawWire(bx, gateY, gatesX + 40, gateY);
                    const dot = this.createElement('circle', {
                        cx: bx, cy: gateY, r: 3.5, fill: '#6366f1'
                    });
                    this.svg.appendChild(dot);
                    termOutputs.push({ x: gatesX + 40, y: gateY });
                }
            }
        });

        // Final Stage: Connect to OR gate or directly to Output F
        const outputX = width - 80;
        const outputY = height / 2;

        if (termOutputs.length === 1) {
            // Only 1 product term: Connect straight to Output F
            const src = termOutputs[0];
            this.drawWire(src.x, src.y, outputX - 25, outputY);
        } else if (termOutputs.length > 1) {
            // Multiple terms: Connect through OR Gate
            const orX = width - 180;
            const orY = height / 2 - 25;
            const orGate = this.drawORGate(orX, orY, 55, 50);
            this.svg.appendChild(orGate);

            // Wire each term output into the OR gate
            const orSpacing = 44 / (termOutputs.length + 1);
            termOutputs.forEach((src, idx) => {
                const targetY = orY + (idx + 1) * orSpacing;
                this.drawSteppedWire(src.x, src.y, orX + 5, targetY);
            });

            // Wire from OR gate output to F
            this.drawWire(orX + 55, orY + 25, outputX - 25, outputY);
        }

        // Draw Output F Node
        this.drawOutputNode(outputX, outputY, 'F');
    }

    renderConstant(val, width, height) {
        const cx = width / 2;
        const cy = height / 2;

        const box = this.createElement('rect', {
            x: cx - 40, y: cy - 25, width: 80, height: 50,
            rx: 12, fill: val === '1' ? '#10b981' : '#64748b',
            filter: 'url(#glow)'
        });
        const text = this.createElement('text', {
            x: cx, y: cy + 8, 'text-anchor': 'middle',
            'font-size': '24', 'font-weight': 'bold', fill: '#ffffff'
        });
        text.textContent = `F = ${val}`;

        const desc = this.createElement('text', {
            x: cx, y: cy + 45, 'text-anchor': 'middle',
            'font-size': '13', fill: '#94a3b8'
        });
        desc.textContent = val === '1' ? 'Tautology (Luôn đúng)' : 'Contradiction (Luôn sai)';

        this.svg.appendChild(box);
        this.svg.appendChild(text);
        this.svg.appendChild(desc);
    }

    drawWire(x1, y1, x2, y2) {
        const line = this.createElement('line', {
            x1, y1, x2, y2,
            stroke: '#94a3b8', 'stroke-width': '2.5', 'stroke-linecap': 'round'
        });
        this.svg.appendChild(line);
    }

    drawSteppedWire(x1, y1, x2, y2) {
        const midX = x1 + (x2 - x1) * 0.55;
        const path = this.createElement('path', {
            d: `M ${x1} ${y1} L ${midX} ${y1} L ${midX} ${y2} L ${x2} ${y2}`,
            fill: 'none', stroke: '#94a3b8', 'stroke-width': '2.5', 'stroke-linejoin': 'round'
        });
        this.svg.appendChild(path);
    }

    drawANDGate(x, y, w, h) {
        const g = this.createElement('g', { filter: 'url(#glow)' });
        const radius = h / 2;
        const straightW = Math.max(w - radius, 5);

        // IEEE Standard AND Shape: flat back, straight top/bottom, semicircular front
        const d = `M ${x} ${y} L ${x + straightW} ${y} A ${radius} ${radius} 0 0 1 ${x + straightW} ${y + h} L ${x} ${y + h} Z`;
        const path = this.createElement('path', {
            d, fill: 'url(#gateGrad)', stroke: '#0284c7', 'stroke-width': '2'
        });
        const label = this.createElement('text', {
            x: x + w * 0.4, y: y + h * 0.58,
            'text-anchor': 'middle', 'font-size': '11', 'font-weight': 'bold', fill: '#ffffff'
        });
        label.textContent = '&';

        g.appendChild(path);
        g.appendChild(label);
        return g;
    }

    drawORGate(x, y, w, h) {
        const g = this.createElement('g', { filter: 'url(#glow)' });
        // IEEE Standard OR Shape: curved back, pointed front
        const d = `M ${x} ${y} Q ${x + w * 0.25} ${y + h * 0.5} ${x} ${y + h} Q ${x + w * 0.6} ${y + h * 0.9} ${x + w} ${y + h * 0.5} Q ${x + w * 0.6} ${y + h * 0.1} ${x} ${y} Z`;
        const path = this.createElement('path', {
            d, fill: 'url(#orGrad)', stroke: '#db2777', 'stroke-width': '2'
        });
        const label = this.createElement('text', {
            x: x + w * 0.42, y: y + h * 0.58,
            'text-anchor': 'middle', 'font-size': '11', 'font-weight': 'bold', fill: '#ffffff'
        });
        label.textContent = '≥1';

        g.appendChild(path);
        g.appendChild(label);
        return g;
    }

    drawNOTGate(x, y, w, h) {
        const g = this.createElement('g', { filter: 'url(#glow)' });
        const bubbleR = 3;
        const triW = w - bubbleR * 2;

        // Triangle body
        const d = `M ${x} ${y} L ${x + triW} ${y + h / 2} L ${x} ${y + h} Z`;
        const tri = this.createElement('path', {
            d, fill: 'url(#notGrad)', stroke: '#d97706', 'stroke-width': '1.5'
        });

        // Inverter bubble
        const bubble = this.createElement('circle', {
            cx: x + triW + bubbleR, cy: y + h / 2, r: bubbleR,
            fill: '#ffffff', stroke: '#d97706', 'stroke-width': '1.5'
        });

        g.appendChild(tri);
        g.appendChild(bubble);
        return g;
    }

    drawOutputNode(x, y, labelText) {
        const g = this.createElement('g', { filter: 'url(#glow)' });
        const outer = this.createElement('circle', {
            cx: x, cy: y, r: 18,
            fill: '#10b981', stroke: '#059669', 'stroke-width': '2'
        });
        const inner = this.createElement('circle', {
            cx: x, cy: y, r: 14,
            fill: 'none', stroke: '#ffffff', 'stroke-width': '1.5', 'stroke-dasharray': '2 2'
        });
        const text = this.createElement('text', {
            x, y: y + 5,
            'text-anchor': 'middle', 'font-size': '14', 'font-weight': 'bold', fill: '#ffffff'
        });
        text.textContent = labelText;

        g.appendChild(outer);
        g.appendChild(inner);
        g.appendChild(text);
        this.svg.appendChild(g);
    }
}

// Export for browser
window.CircuitRenderer = CircuitRenderer;

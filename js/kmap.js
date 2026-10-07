/**
 * K-Map & Boolean Minimization Engine (JavaScript Port)
 * Supports 2, 3, and 4 variables (SOP Minimization, Prime Implicants, Minimal Covers)
 * Group 8 - Discrete Mathematics & Graph Theory (HCMUTE)
 */

class KMapEngine {
    constructor() {
        this.grayCodes1 = ['0', '1'];
        this.grayCodes2 = ['00', '01', '11', '10'];
    }

    /**
     * Extracts unique variables from expression, normalizes to uppercase.
     */
    extractVariables(expr) {
        const set = new Set();
        for (let ch of expr) {
            if (/[a-zA-Z]/.test(ch)) {
                set.add(ch.toUpperCase());
            }
        }
        let vars = Array.from(set).sort();
        if (vars.length < 2) {
            if (vars.length === 0) vars = ['A', 'B'];
            else if (vars[0] === 'A') vars = ['A', 'B'];
            else vars = ['A', vars[0]].sort();
        }
        if (vars.length > 4) {
            vars = vars.slice(0, 4); // Limit to 4 variables for standard K-map
        }
        return vars;
    }

    /**
     * Parses SOP expression into a set of covered minterm bitstrings (e.g., "0110")
     */
    parseExpressionToMinterms(expr, variables) {
        const n = variables.length;
        const mintermSet = new Set();
        const terms = expr.split('+');

        for (let rawTerm of terms) {
            const term = rawTerm.trim();
            if (!term) continue;

            if (term === '1') {
                const total = 1 << n;
                for (let m = 0; m < total; m++) {
                    mintermSet.add(m.toString(2).padStart(n, '0'));
                }
                continue;
            }
            if (term === '0') continue;

            // val[k]: 0 = negated, 1 = true, -1 = don't care
            const val = new Array(n).fill(-1);
            let commonNeg = false;

            for (let i = 0; i < term.length; i++) {
                const ch = term[i];
                if (ch === '(' && i > 0 && term[i - 1] === '-') {
                    commonNeg = true;
                } else if (ch === ')') {
                    commonNeg = false;
                } else if (/[a-zA-Z]/.test(ch)) {
                    const varChar = ch.toUpperCase();
                    const varIdx = variables.indexOf(varChar);
                    if (varIdx !== -1) {
                        let isNeg = commonNeg;
                        if (!isNeg) {
                            let j = i - 1;
                            while (j >= 0 && /\s/.test(term[j])) j--;
                            if (j >= 0 && (term[j] === '-' || term[j] === '~' || term[j] === '!')) {
                                isNeg = true;
                            }
                        }
                        // Also support apostrophe after variable: A'
                        let next = i + 1;
                        while (next < term.length && /\s/.test(term[next])) next++;
                        if (next < term.length && term[next] === "'") {
                            isNeg = true;
                        }

                        val[varIdx] = isNeg ? 0 : 1;
                    }
                }
            }

            // Expand literals to matching minterms
            const total = 1 << n;
            for (let m = 0; m < total; m++) {
                let match = true;
                for (let k = 0; k < n; k++) {
                    const bit = (m >> (n - 1 - k)) & 1;
                    if (val[k] !== -1 && val[k] !== bit) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    mintermSet.add(m.toString(2).padStart(n, '0'));
                }
            }
        }

        return Array.from(mintermSet);
    }

    /**
     * Map bitstring to grid coordinates (row, col)
     */
    bitToCoords(bitString, n) {
        const split = Math.floor(n / 2);
        const rowBits = bitString.slice(0, split);
        const colBits = bitString.slice(split);

        const r = split === 0 ? 0 : this.grayToIndex(rowBits);
        const c = (n - split) === 0 ? 0 : this.grayToIndex(colBits);
        return { r, c };
    }

    grayToIndex(gray) {
        if (gray.length === 1) return gray === '1' ? 1 : 0;
        if (gray.length === 2) return this.grayCodes2.indexOf(gray);
        return 0;
    }

    indexToGray(idx, len) {
        if (len === 1) return idx === 1 ? '1' : '0';
        if (len === 2) return this.grayCodes2[idx] || '00';
        return '';
    }

    /**
     * Builds 2D grid matrix of size 2^(n/2) x 2^(n - n/2)
     */
    buildGrid(minterms, n) {
        const rowCount = 1 << Math.floor(n / 2);
        const colCount = 1 << (n - Math.floor(n / 2));
        const grid = Array.from({ length: rowCount }, () => new Array(colCount).fill(0));

        for (let bits of minterms) {
            const { r, c } = this.bitToCoords(bits, n);
            if (r >= 0 && r < rowCount && c >= 0 && c < colCount) {
                grid[r][c] = 1;
            }
        }
        return grid;
    }

    /**
     * Generates all cell coordinates occupied by a group of size (rSize, cSize) at (sRow, sCol)
     */
    getGroupCells(sRow, sCol, rSize, cSize, rowCount, colCount) {
        const cells = [];
        for (let i = 0; i < rSize; i++) {
            for (let j = 0; j < cSize; j++) {
                cells.push({
                    r: (sRow + i) % rowCount,
                    c: (sCol + j) % colCount
                });
            }
        }
        return cells;
    }

    /**
     * Checks if a group is completely filled with 1s
     */
    isGroupValid(grid, sRow, sCol, rSize, cSize) {
        const rowCount = grid.length;
        const colCount = grid[0].length;
        for (let i = 0; i < rSize; i++) {
            for (let j = 0; j < cSize; j++) {
                const r = (sRow + i) % rowCount;
                const c = (sCol + j) % colCount;
                if (!grid[r][c]) return false;
            }
        }
        return true;
    }

    cellKey(cell) {
        return `${cell.r},${cell.c}`;
    }

    isSubset(subCells, superCells) {
        const superSet = new Set(superCells.map(this.cellKey));
        return subCells.every(c => superSet.has(this.cellKey(c)));
    }

    areCellSetsEqual(a, b) {
        if (a.length !== b.length) return false;
        const setA = new Set(a.map(this.cellKey));
        return b.every(c => setA.has(this.cellKey(c)));
    }

    /**
     * Finds all Prime Implicants (PI)
     */
    findPrimeImplicants(grid) {
        const rowCount = grid.length;
        const colCount = grid[0].length;
        const allGroups = [];

        // Generate candidate dimensions (powers of 2)
        const dimensions = [];
        for (let r = rowCount; r >= 1; r = Math.floor(r / 2)) {
            for (let c = colCount; c >= 1; c = Math.floor(c / 2)) {
                dimensions.push({ rSize: r, cSize: c, area: r * c });
            }
        }
        dimensions.sort((a, b) => b.area - a.area);

        for (let dim of dimensions) {
            const { rSize, cSize } = dim;
            for (let sRow = 0; sRow < rowCount; sRow++) {
                for (let sCol = 0; sCol < colCount; sCol++) {
                    if (this.isGroupValid(grid, sRow, sCol, rSize, cSize)) {
                        const cells = this.getGroupCells(sRow, sCol, rSize, cSize, rowCount, colCount);
                        let skip = false;

                        for (let existing of allGroups) {
                            if (this.areCellSetsEqual(existing.cells, cells) || this.isSubset(cells, existing.cells)) {
                                skip = true;
                                break;
                            }
                        }

                        if (!skip) {
                            allGroups.push({
                                sRow,
                                sCol,
                                rSize,
                                cSize,
                                cells
                            });
                        }
                    }
                }
            }
        }
        return allGroups;
    }

    /**
     * Converts a cell coordinate back to variable bit string
     */
    coordsToBits(r, c, n) {
        const split = Math.floor(n / 2);
        const rowGray = this.indexToGray(r, split);
        const colGray = this.indexToGray(c, n - split);
        return rowGray + colGray;
    }

    /**
     * Converts a group to Boolean product term (e.g. "A -B D" or "1")
     */
    groupToTerm(group, variables) {
        const n = variables.length;
        if (!group.cells.length) return '0';

        const firstBits = this.coordsToBits(group.cells[0].r, group.cells[0].c, n).split('');
        for (let i = 1; i < group.cells.length; i++) {
            const bits = this.coordsToBits(group.cells[i].r, group.cells[i].c, n);
            for (let k = 0; k < n; k++) {
                if (firstBits[k] !== 'x' && firstBits[k] !== bits[k]) {
                    firstBits[k] = 'x';
                }
            }
        }

        let term = '';
        for (let k = 0; k < n; k++) {
            if (firstBits[k] === '0') {
                term += `-${variables[k]} `;
            } else if (firstBits[k] === '1') {
                term += `${variables[k]} `;
            }
        }
        term = term.trim();
        return term === '' ? '1' : term;
    }

    /**
     * Minimal SOP cover using Petrick's Method / Branch & Bound
     */
    solveMinimalCovers(grid, PIs, variables) {
        const ones = [];
        for (let r = 0; r < grid.length; r++) {
            for (let c = 0; c < grid[0].length; c++) {
                if (grid[r][c] === 1) {
                    ones.push({ r, c });
                }
            }
        }

        if (ones.length === 0) {
            return [{ groups: [], expression: '0' }];
        }

        // Check if map is all 1s
        if (ones.length === grid.length * grid[0].length && PIs.length > 0) {
            const fullGroup = PIs.find(g => g.cells.length === ones.length);
            if (fullGroup) {
                return [{ groups: [fullGroup], expression: '1' }];
            }
        }

        // Coverage table: cover[oneIndex] = [piIndex...]
        const cover = ones.map(one => {
            const coveredBy = [];
            PIs.forEach((pi, piIdx) => {
                if (pi.cells.some(c => c.r === one.r && c.c === one.c)) {
                    coveredBy.push(piIdx);
                }
            });
            return coveredBy;
        });

        // 1. Identify Essential Prime Implicants (EPIs)
        const epiSet = new Set();
        cover.forEach(coveredBy => {
            if (coveredBy.length === 1) {
                epiSet.add(coveredBy[0]);
            }
        });

        const epiIndices = Array.from(epiSet);
        const coveredOnes = new Set();
        epiIndices.forEach(epi => {
            cover.forEach((coveredBy, oneIdx) => {
                if (coveredBy.includes(epi)) {
                    coveredOnes.add(oneIdx);
                }
            });
        });

        const uncoveredOnes = [];
        for (let i = 0; i < ones.length; i++) {
            if (!coveredOnes.has(i)) uncoveredOnes.push(i);
        }

        const nonEpiPIs = [];
        PIs.forEach((_, idx) => {
            if (!epiSet.has(idx)) nonEpiPIs.push(idx);
        });

        // If all 1s are covered by EPIs
        if (uncoveredOnes.length === 0) {
            const chosen = epiIndices.map(i => PIs[i]);
            const expr = this.groupsToExpression(chosen, variables);
            return [{ groups: chosen, expression: expr }];
        }

        // 2. Branch & Bound / Bitmask for remaining uncovered 1s
        const R = nonEpiPIs.length;
        const total = 1 << R;
        let bestCost = [Infinity, Infinity]; // [countOfPIs, countOfLiterals]
        let bestSolutions = [];

        const countLiterals = (chosenIndices) => {
            let lits = 0;
            chosenIndices.forEach(idx => {
                const term = this.groupToTerm(PIs[idx], variables);
                const letters = (term.match(/[A-Za-z]/g) || []).length;
                lits += letters;
            });
            return lits;
        };

        for (let mask = 0; mask < total; mask++) {
            const bitsCount = mask.toString(2).replace(/0/g, '').length;
            if (bitsCount > bestCost[0]) continue;

            let allCovered = true;
            for (let oneIdx of uncoveredOnes) {
                let covered = false;
                for (let b = 0; b < R; b++) {
                    if (mask & (1 << b)) {
                        const piIdx = nonEpiPIs[b];
                        if (cover[oneIdx].includes(piIdx)) {
                            covered = true;
                            break;
                        }
                    }
                }
                if (!covered) {
                    allCovered = false;
                    break;
                }
            }
            if (!allCovered) continue;

            const selected = [];
            for (let b = 0; b < R; b++) {
                if (mask & (1 << b)) selected.push(nonEpiPIs[b]);
            }

            const totalChosen = [...epiIndices, ...selected];
            const cost = [totalChosen.length, countLiterals(totalChosen)];

            if (cost[0] < bestCost[0] || (cost[0] === bestCost[0] && cost[1] < bestCost[1])) {
                bestCost = cost;
                bestSolutions = [totalChosen];
            } else if (cost[0] === bestCost[0] && cost[1] === bestCost[1]) {
                bestSolutions.push(totalChosen);
            }
        }

        const out = [];
        const seen = new Set();
        bestSolutions.forEach(solIndices => {
            const chosenGroups = solIndices.map(i => PIs[i]);
            const expr = this.groupsToExpression(chosenGroups, variables);
            if (!seen.has(expr)) {
                seen.add(expr);
                out.push({ groups: chosenGroups, expression: expr });
            }
        });

        return out.length ? out : [{ groups: epiIndices.map(i => PIs[i]), expression: this.groupsToExpression(epiIndices.map(i => PIs[i]), variables) }];
    }

    groupsToExpression(groups, variables) {
        if (!groups.length) return '0';
        const terms = Array.from(new Set(groups.map(g => this.groupToTerm(g, variables))));
        if (terms.includes('1')) return '1';
        return terms.join(' + ');
    }
}

// Export for browser
window.KMapEngine = KMapEngine;

export default class MultiMap {
    map = null;

    constructor(multimap) {
        if (multimap) {
            this.map = new Map(multimap.map);
        } else {
            this.map = new Map();
        }
    }

    [Symbol.iterator]() {
        return this.map.iterator;
    };

    /**
     * Adds a value to the key array
     * @param k
     * @param v
     */
    add(k, v) {
        if (!this.map.has(k)) {
            this.map.set(k, [v]);
        } else {
            this.map.get(k).push(v);
        }
        return this;
    }

    set(k, v) {
        this.map.delete(k);
        this.map.set(k, [v]);
        return this;
    }

    get(k) {
        if (this.map.has(k)) {
            return this.map.get(k);
        } else {
            return [];
        }
    }

    has(k) {
        return this.map.has(k) && this.get(k).length > 0;
    }

    contains(k, equalityfn = () => true) {
        return this.map.has(k) && this.get(k).filter(equalityfn).length > 0;
    }

    entries() {
        return this.map.entries();
    }

    isEmpty() {
        for (const [k, v] of this.entries()) {
            if (v.length > 0) {
                return false;
            }
        }
        return true;
    }

    asArray() {
        if (this.map.size > 0) {
            return [...this.map.entries()];
        } else {
            return [];
        }
    }

    removeAll(k, equalityfn = () => true) {
        const hits = this.get(k).filter(equalityfn);
        const rslt = [];
        for (const hit of hits) {
            rslt.push(...this.remove(k, hit));
        }
        return rslt;
    }

    remove(k, v) {
        let idx;
        let rslt = [];
        while ((idx = this.get(k).indexOf(v)) > -1) {
            rslt.push(this.get(k).splice(idx, 1));
        }
        return rslt;
    }

    size() {
        return this.asArray().length;
    }
}

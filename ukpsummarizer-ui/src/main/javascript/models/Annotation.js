export default class Annotation {

    constructor(pointerindex, start, end, value = "accept", concept = "") {
        this.index = pointerindex;
        this.start = start;
        this.end = end;
        this.value = value;
        this.concept = concept;
    }

    /**
     * returns true iff the other annotation is completly covered by this, i.e. other.start > this.start && other.end <= this.end
     * @param other
     */
    covers(other) {
        if (this.index !== other.index) return false;
        return other.start >= this.start && other.end <= this.end;
    }

    intersects(other) {
        if (this.index !== other.index) return false;
        if (this.start >= other.end) return false;
        if (this.end <= other.start) return false;
        return true;
    }

    touches(other) {
        if (this.index !== other.index) return false;
        if (this.start === other.end) return true;
        if (this.end === other.start) return true;
        return false;
    }

    setConcept(concept = "") {
        this.concept = concept;
        return this;
    }

}

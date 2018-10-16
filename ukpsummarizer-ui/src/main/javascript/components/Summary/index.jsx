/**
 * Created by hatieke on 2017-02-05.
 */

import React from 'react';
import {Popover, ButtonGroup, Button, Overlay} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import Json from '../JsonComponent';
import './styles.less';
import Annotation from "../../models/Annotation";

export default class Summary extends React.Component {
    constructor(...args) {
        super(...args);
        this.keyPress = this.keyPress.bind(this);
    }

    state = {
        annotations: []
    };

    static defaultProps = {
        callback: null,
        footer: "",
        content: [],
        annotations: []
    };

    keyPress(evt) {
        console.log("keypress", evt);

        switch (evt.code) {
            case "KeyA":
                this._acceptSelection("accept");
                break;
            case "KeyR":
                this._acceptSelection("reject");
                break;
            case "KeyC":
                this._acceptSelection("remove");
                break;
        }
    };

    _acceptSelection(value = "new") {

        let previousAnnotations = (this.props.callback === null && this.state.annotations.length > 0) ? this.state.annotations : this.props.annotations;

        const selection = document.getSelection();
        if (selection.isCollapsed) {
            return;
        }

        const range = selection.getRangeAt(0);
        if (!range.intersectsNode(this.container)) {
            return;
        }

        function minimizeRange(r, paragraph) {

            let newRange = document.createRange();
            newRange.selectNode(paragraph);
            if (paragraph.previousSibling !== null && r.intersectsNode(paragraph.previousSibling)) {
                newRange.setStart(paragraph.firstChild, 0);
            } else {
                newRange.setStart(r.startContainer, r.startOffset);
            }

            if (paragraph.nextSibling !== null && r.intersectsNode(paragraph.nextSibling)) {
                if (paragraph.lastChild.lastChild !== undefined && paragraph.lastChild.lastChild !== null) {
                    newRange.setEnd(paragraph.lastChild.lastChild, paragraph.lastChild.lastChild.textContent.length);
                } else {
                    newRange.setEnd(paragraph.lastChild, paragraph.lastChild.textContent.length);
                }
            } else {
                newRange.setEnd(r.endContainer, r.endOffset);

            }

            // if (paragraph === newRange.commonAncestorContainer) {
            //     console.log("common ancestor");
            // } else {
            //     console.log("different ancestor", paragraph, newRange.commonAncestorContainer);
            // }
            return newRange;
        }

        /**
         * walks through the dom tree that is formed by the paragraph until range.startContainer is found.
         * For each node except the startContainer, we add the actual textcontent to the start- and end-offset. In the last sibling, we merely add the range-offsets
         *
         * @param range
         * @param paragraph
         */
        function resolveRangeToAnnotationOffsets(range, paragraph, resolve_to = "start") {
            let pointer = paragraph.firstChild;
            let offset = 0;

            // while (pointer !== null && !range.intersectsNode(pointer)) {
            let resolve_pointer = null;
            switch (resolve_to) {
                case "start":
                default:
                    resolve_pointer = range.startContainer;
                    break;
                case "end":
                    resolve_pointer = range.endContainer;
                    break;
            }
            while (pointer !== null && pointer !== resolve_pointer && pointer !== resolve_pointer.parentNode) {
                offset += pointer.textContent.length;
                pointer = pointer.nextSibling;
            }

            switch (resolve_to) {
                case "start":
                    offset += range.startOffset;
                    break;
                case "end":
                    offset += range.endOffset
            }
            return offset;
        }


        /**
         * iterate over all paragraphs (i.e. children of this.container) and check if it covers the range, and split the
         * ranges accordingly
         */
        /**
         * each range now has to be added to the existing annotations. We need to split and merge the existing annotations,
         * so that the new range can be converted into an annotation.
         */
        let annotations = [];

        /**
         * convert current selection into new annotations
         */
        for (let paragraphIndex = 0; paragraphIndex < this.container.childElementCount; paragraphIndex++) {
            let paragraph = this.container.childNodes[paragraphIndex];
            if (range.intersectsNode(paragraph)) {
                let relevantRange = minimizeRange(range, paragraph);
                let startOffset = resolveRangeToAnnotationOffsets(relevantRange, paragraph, "start");
                let endOffset = resolveRangeToAnnotationOffsets(relevantRange, paragraph, "end");
                if (relevantRange.collapsed) {
                    continue;
                }
                annotations.push(new Annotation(this.props.content[paragraphIndex].sentenceSubsetIndex, startOffset, endOffset, value));
            }
        }

        /**
         * After adding a annotation, we have to throw out those that are completely covered by this annotation.
         *
         * merge annotations, so that each paragraph has a non-overlapping collection of annotations.
         * Most important thing is, that the new annotations overwrite the older ones.
         * If adjacent annotations have the same label (value), then merge them.
         *
         */

        let mergedAnnotations = [];

        this.props.content.forEach((sentence) => {
            const old_match = previousAnnotations.filter(annotation => annotation.index === sentence.sentenceSubsetIndex);
            const new_match = annotations.filter(annotation => annotation.index === sentence.sentenceSubsetIndex);

            if (new_match.length === 0) {
                Array.prototype.push.apply(mergedAnnotations, old_match);
                return;
            } else if (new_match.length > 1) {
                console.warn("more than one new annotation for that paragraph.!", new_match, sentence.sentenceSubsetIndex);
            }
            for (let i = 0; i < new_match.length; i++) {
                const na = new_match[i];
                for (let j = 0; j < old_match.length; j++) {
                    const oa = old_match[j];
                    if (na.covers(oa)) {
                        // if oa is completely covered by na, then delete the annotation.
                    } else if (na.intersects(oa)) {
                        let convertedOa = [];
                        if (oa.start < na.start) {
                            const a = new Annotation(na.index, oa.start, na.start, oa.value);
                            convertedOa.push(a);
                        }
                        if (oa.end > na.end) {
                            const a = new Annotation(na.index, na.end, oa.end, oa.value);
                            convertedOa.push(a);
                        }

                        Array.prototype.push.apply(mergedAnnotations, convertedOa);
                    } else {
                        mergedAnnotations.push(oa);
                    }
                }
                if (na.value !== "remove") {
                    mergedAnnotations.push(na);
                }
            }
        });

        function compress(input) {
            function isCompressable(array) {
                for (let i = 0; i < array.length; i++) {
                    for (let j = 0; j < i; j++) {
                        if (i === j) {
                            continue;
                        }
                        const a1 = array[i];
                        const a2 = array[j];

                        if (a1.index !== a2.index)
                            continue;

                        if (a1.value !== a2.value)
                            continue;

                        if (a1.intersects(a2)) {
                            return {
                                a: a1,
                                b: a2
                            };
                        } else if (a1.touches(a2)) {
                            return {
                                a: a1,
                                b: a2
                            };
                        }
                    }
                }
                return null;
            }

            let result = Array.from(input);

            let candidates = null;
            while (candidates = isCompressable(result)) {
                const {a, b} = candidates;
                const mergedAnnotation = new Annotation(a.index, Math.min(a.start, b.start), Math.max(a.end, b.end), a.value);
                let index_a = result.indexOf(a);
                if (index_a >= 0) {
                    result.splice(index_a, 1);
                }
                let index_b = result.indexOf(b);
                if (index_b >= 0) {
                    result.splice(index_b, 1);
                }
                result.push(mergedAnnotation);
            }

            return result;
        }

        /**
         * Validate that the mergedAnnotations do not intersect.
         * Merge annotations if they have same borders AND same value.
         */

        let finalAnnotations = compress(mergedAnnotations);

        document.getSelection().collapseToEnd();
        this.callbackHandler({
            sentences: this.props.content.map(e => e.sentenceSubsetIndex),
            annotations: finalAnnotations
        });
        // this.setState({
        //     // selection: document.getSelection(),
        //     annotations: finalAnnotations
        // });
    }

    componentWillMount() {
        // register selection hooks
        this.selectionChangeHook = () => {

            const selection = document.getSelection();
            if (selection.isCollapsed) {
                this.setState({
                    showPopover: false
                });
                return;
            }
            if (selection.rangeCount < 1) {
                this.setState({
                    showPopover: false
                });
                return;
            }

            const range = selection.getRangeAt(0);
            if (!range.intersectsNode(this.container)) {
                this.setState({
                    showPopover: false
                });
                return;
            }

            this.setState({
                showPopover: true
            });
        };
        document.addEventListener("selectionchange", this.selectionChangeHook);
        document.addEventListener("keypress", this.keyPress);
    }

    componentWillUnmount() {
        document.removeEventListener("selectionchange", this.selectionChangeHook);
        document.removeEventListener("keypress", this.keyPress);
    }

    render() {
        const {footer, content} = this.props;
        const {showPopover} = this.state;

        const annotations = (this.props.callback === null && this.state.annotations.length > 0) ? this.state.annotations : this.props.annotations;

        let po = null;
        if (showPopover) {
            /**
             * draw the popover.
             */
            const selection = document.getSelection();
            let boundingClientRect = selection.getRangeAt(0).getBoundingClientRect();
            // let top_pos = selection.getRangeAt(0).startContainer.parentElement.offsetTop - poheight;
            let left_pos = boundingClientRect.width / 2 + boundingClientRect.left;
            let top_pos = boundingClientRect.top / 2;
            let item = "";

            let poTarget = selection.getRangeAt(0).startContainer;
            // let poTarget = selection.focusNode;
            // let poTarget = this.container;
            while (poTarget.nodeType !== 1) {
                poTarget = poTarget.parentNode;
            }
            // container={this.container}
            po = (<Overlay
                show={showPopover}
                target={poTarget}
                placement="top"
            >
                <Popover
                    style={{minWidth: "400px"}}
                    id="nobody-cares"
                    title="Is the selected text relevant?"
                >
                    <ButtonGroup>
                        <Button className="accept" onClick={this._acceptSelection.bind(this, "accept")}>
                            <FontAwesome size='2x' name="thumbs-o-up"/>Yes</Button>
                        <Button className="reject" onClick={this._acceptSelection.bind(this, "reject")}>
                            <FontAwesome size='2x' name="thumbs-o-down"/>No</Button>
                        <Button onClick={this._acceptSelection.bind(this, "remove")}>
                            <FontAwesome size='2x' name="eraser"/>Erase</Button>
                    </ButtonGroup>
                </Popover>
            </Overlay>);
        }

        const sentences = content.map((o, i) => {
            const a = annotations.filter(e => e.index === o.sentenceSubsetIndex);
            const muted = (o.hasOwnProperty("muted")) ? o["muted"] : false;
            return (<AnnotatedSentence key={o.sentenceSubsetIndex}
                                       sentence={o}
                                       muted={muted}
                                       annotations={a}/>);
        });

        return (<div>
            <blockquote>
                <div ref={(el) => {
                    this.container = el;
                }}>
                    {sentences}
                </div>
                <footer>{footer}</footer>
            </blockquote>
            {po}
        </div>)
    }


    callbackHandler(data = {sentences: [], annotations: []}) {
        if (this.props.callback !== null) {
            this.props.callback(data);
        } else {
            this.setState({
                sentences: data.sentences,
                annotations: data.annotations
            });
        }
    }
}

class AnnotatedSentence extends React.Component {
    static defaultProps = {
        sentence: {
            "concepts": [
                "string"
            ],
            "doc_id": 0,
            "id": 0,
            "length": 0,
            "phrases": [
                "string"
            ],
            "sent_id": 0,
            "sentenceSubsetIndex": 0,
            "tokens": [
                "string"
            ],
            "untokenized_concepts": [
                "string"
            ],
            "untokenized_form": "string",
            "untokenized_phrases": [
                "string"
            ]
        },
        muted: false,
        annotations: []
    };

    createMarkup() {
        let p = document.createElement("p");
        let newContent = document.createTextNode(this.props.sentence.untokenized_form);
        p.appendChild(newContent); //add the text node to the newly created div.

        let ranges = [];
        let orderedAnnotationsMightHelp = Array.from(this.props.annotations);
        orderedAnnotationsMightHelp.sort((a, b) => b.start - a.start);
        for (let i = 0; i < orderedAnnotationsMightHelp.length; i++) {
            const annotation = orderedAnnotationsMightHelp[i];
            let range = document.createRange();

            range.setStart(newContent, annotation.start);
            range.setEnd(newContent, annotation.end);
            ranges.push({r: range, v: annotation.value, id: annotation.start});
        }

        ranges.forEach(r => {
            let element = document.createElement("mark");
            element.className += " " + r.v;
            element.id = "" + r.id;
            r.r.surroundContents(element);
        });

        return {__html: p.innerHTML};
    }

    componentDidUpdate(props, state) {

    }

    render() {
        return (<p className={this.props.muted ? "text-muted" : ""}
            // ref={(el) => {this.container = el}}
                   dangerouslySetInnerHTML={this.createMarkup()}
        />);
    }
}


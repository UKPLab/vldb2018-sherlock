/**
 * Hendrik Lücke-Tieke
 * dataexpedition.net
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 *
 * Do not use without prior consent by the copyright holder.
 *
 **/
import React from 'react';
import {connect} from 'react-redux';
import {bindActionCreators} from 'redux';
import * as DatasetActions from '../../controllers/datasets/actions';
import * as AssignmentActions from '../../controllers/assignment/actions';
import * as SummaryActions from '../../controllers/summary/actions';
import Json from '../../components/JsonComponent';
import _ from 'lodash';

import routeConfig from '../../config/routes';

import {
    Grid,
    Row,
    Col,
    Button,
    ButtonGroup,
    ButtonToolbar,
    FormControl,
    FormGroup,
    ControlLabel,
    Label,
    Jumbotron,
    Panel,
    Well,
    Tooltip,
    OverlayTrigger,
    Tabs,
    Tab,
    PageHeader,
    Clearfix,
    Alert, ListGroup, ListGroupItem, Table
} from 'react-bootstrap';
import {LinkContainer} from "react-router-bootstrap";
import FontAwesome from 'react-fontawesome';
import Summary from '../../components/Summary';
import FeedbackComponent from '../../components/FeedbackComponent';

import Spinner from '../../components/Spinner';
import './style.less';
import Annotation from "../../models/Annotation";
import MultiMap from "../../models/MultiMap";
import TopicCollectionComponent from "../../components/TopicCollectionComponent";
import SubmitControlPanel from "../../components/SubmitControlPanel";

/**
 * the sentence.untokenized_concepts are NOT sorted!
 * @param sentence
 * @param index
 * @returns {Array}
 */
function extractConceptAnnotations(sentence) {
    let s = sentence.untokenized_form.toLowerCase();
    const c = sentence.untokenized_concepts;

    let result = [];
    for (let i = 0; i < c.length; i++) {
        const concept = c[i];
        const startPosition = s.indexOf(concept);
        if (startPosition < 0) {
            continue;
        }
        const endPosition = startPosition + concept.length;
        const anno = new Annotation(sentence.sentenceSubsetIndex, startPosition, endPosition, "concept").setConcept(sentence.concepts[i]);
        console.log(concept, c);
        result.push(anno);
    }
    console.log(result);
    return result;
}

/**
 * This container is a subapp which enables interactive summarization for a given topic.
 */
class AnalysisContainer extends React.Component {
    state = {
        annotations: new MultiMap(),
        interactions: new Map(),
        k_size: 0.1
    };

    constructor(...args) {
        super(...args);
        this.changeInteraction = this.changeInteraction.bind(this);
    }

    componentDidMount() {
        window.scrollTo(0, 0);
    }

    componentWillReceiveProps(nextProps) {
        this.setState({
            annotations: new MultiMap(),
            interactions: new Map()
        });

        /**  if(this.props.store.users.loaded === false && nextProps.store.users.loaded === true){
            nextProps.actions.loadSummary(nextProps.store.users.user.id, 0, nextProps.location.query.topic);
        }**/
    }

    onPickK(e) {
        console.log('[onPickK]', this.inputEl.value)
        this.setState({ k_size: this.inputEl.value });
    }

    __save_feedback__() {
        const {interactions} = this.state;
        const result = Array.of(...interactions.values());
        this.props.actions.saveFeedback(result);
    }

    pullStateFromFeedbackComponent(data) {
        const {interactions} = this.state;

        // console.log(data);
        //merge data into state;
        let newMap = new Map(interactions);
        data.forEach(i => {
            if (i.value == "recommendation") {
                if (newMap.has(i.concept)) {
                    newMap.delete(i.concept);
                }
            } else {
                newMap.set(i.concept, i);
            }
        });

        this.setState({
            interactions: newMap
        });

    }

    /**
     *
     * @param concept string (id) of concept/interaction to change
     * @param value the new value of that interaction
     */
    changeInteraction(concept, value = "recommendation") {
        console.log("Trying to change concept ", concept, "to value", value);
        const {store: {assignmentTemplates, assignment}} = this.props;
        const {annotations, interactions} = this.state;

        if (!interactions.has(concept)) {
            return;
        }
        const pysentences = assignmentTemplates.get(assignment.data.assignmentTemplate).data.sentences;

        const interaction = interactions.get(concept);

        /**
         * change Annotation a of this.state.annotations
         * where  pysentence.sentenceSubsetIndex === a.index
         *  AND pysentence.concepts contains concept.
         */

        const newAnnotations = new MultiMap(annotations);
        // const deletions = [];
        // const replacements = [];
        for (const entry of annotations.entries()) {
            const key = entry[0];
            for (const annotation of entry[1]) {
                const psss = pysentences.filter(s => s.sentenceSubsetIndex === annotation.index)
                    .filter(s => s.concepts.indexOf(concept) > -1);
                for (const sentence of psss) {
                    const pysentence_annotations = extractConceptAnnotations(sentence).filter(a => annotation.intersects(a))
                        .filter(a => a.concept === concept);

                    if (pysentence_annotations.length > 0) {
                        for (const pa of pysentence_annotations) {
                            if (pa.value === annotation.value) {
                                return;
                            }
                            if (value !== "recommendation") {
                                // replacements.push(pa);
                                const convertedAnnotation = new Annotation(pa.index, pa.start, pa.end, value, pa.concept);
                                newAnnotations.add(key, convertedAnnotation);
                            }

                            // console.log("pa intersects annotation:", pa, annotation);
                            // pa has to be substracted from annotation.

                            // deletions.push(annotation);
                            if (pa.covers(annotation)) {
                                // if annotation is completely covered by pa, then delete the annotation.
                                newAnnotations.remove(key, annotation);

                            } else if (pa.intersects(annotation)) {
                                newAnnotations.remove(key, annotation);

                                // let convertedOa = [];
                                if (annotation.start < pa.start) {
                                    const a = new Annotation(pa.index, annotation.start, pa.start, annotation.value);
                                    // convertedOa.push(a);
                                    newAnnotations.add(key, a);
                                }
                                if (annotation.end > pa.end) {
                                    const a = new Annotation(pa.index, pa.end, annotation.end, annotation.value);
                                    // convertedOa.push(a);
                                    newAnnotations.add(key, a);
                                }
                            }
                        }
                    }
                }
            }
        }
        // console.log(deletions, replacements);
        for (const entry of newAnnotations.entries()) {
            newAnnotations.removeAll(entry[0], (v) => v.value === "concept");
        }

        const feedbackAnnotations = this.convertRawAnnotationsToInteractions(pysentences, newAnnotations);
        const latestIteration = assignment.data.iterations.reduce((a, b) => (b.iteration > a.iteration) ? b : a);

        const newInteractionsMap = new Map();
        feedbackAnnotations.forEach(f => {
            newInteractionsMap.set(f.concept, {
                iteration: latestIteration.iteration,
                ...f
            });
        });


        this.setState({
            annotations: newAnnotations,
            interactions: newInteractionsMap
        });
    }

    /**
     *
     * @param content: arbitrary list of pysentences
     * @param annotations: arbitrary list of annotations
     */
    convertRawAnnotationsToInteractions(content, annotations) {

        let feedback = [];


        for (const pysentence of content) {
            if (!annotations.has(pysentence.sentenceSubsetIndex)) {
                continue;
            }

            // convert each concept into an annotation, and make use of the fact, that they are ordered.
            const concept_annotations = extractConceptAnnotations(pysentence);

            let sortedAnnotations = annotations.get(pysentence.sentenceSubsetIndex).sort((a, b) => a.start - b.start);

            for (const annotation of sortedAnnotations) {
                // iterate over the untokenized_concepts in the pysentence one by one, and check if the sorted annotations covers it. if yes, add to result pool,
                // otherwise go to next
                const substring = pysentence.untokenized_form.substring(annotation.start, annotation.end);

                const coveredAnnotations = concept_annotations.filter(a => annotation.covers(a));
                coveredAnnotations.forEach(e => {
                    const concept_string = pysentence.untokenized_form.substring(e.start, e.end);
                    // console.log("covered", concept_string, substring);

                    feedback.push(new Annotation(annotation.sentenceSubsetIndex, e.start, e.end, annotation.value).setConcept(e.concept));
                });
            }
        }


        // at last, convert the annotations into interactions:


        return feedback;
    }

    pullStateFromSummary({annotations, sentences}) {
        const {store: {summary, assignment, assignmentTemplates}} = this.props;
        const {annotations: oldAnnotations, interactions: oldInteractions} = this.state;

        const latestIteration = assignment.data.iterations.reduce((a, b) => (b.iteration > a.iteration) ? b : a);

        let newAnnos = new MultiMap(oldAnnotations);
        for (const idx of sentences) {
            newAnnos.removeAll(idx);
        }

        annotations.forEach(a => {
            const k = a.index;
            newAnnos.add(k, a);
        });

        const assignmentTemplate = assignmentTemplates.get(assignment.data.assignmentTemplate);

        const feedbackAnnotations = this.convertRawAnnotationsToInteractions(assignmentTemplate.data.sentences, newAnnos);
        const newInteractionsMap = new Map();
        feedbackAnnotations.forEach(f => {
            newInteractionsMap.set(f.concept, {
                iteration: latestIteration.iteration,
                ...f
            });
        });


        this.setState({
            annotations: newAnnos,
            interactions: newInteractionsMap
        });
    }

    render() {
        console.log("Render AnalysisContainer");
        const {store: {assignment, summary, assignmentTemplates}, actions, location, routeParams} = this.props;

        let narrative = assignment.data.narrative;
        let title = assignment.data.title;
        const latestIteration = assignment.data.iterations.reduce((a, b) => (b.iteration > a.iteration) ? b : a);

        let fullText = [];
        let confirmatory_summary_component = <Well>Summary not available</Well>;
        let exploratory_summary_component = <Well>Summary not available</Well>;
        switch (summary.summary.state) {
            case "LOADING":
            case "NEW":
                let narrative = assignment.data.narrative;
                let title = assignment.data.title;

                confirmatory_summary_component = <Spinner text="Please wait... loading summary"/>;
                exploratory_summary_component = <Spinner text="Please waiti... loading summary"/>;
                break;
            case "LOADED":
                let confirmatory_sentences = [];
                let exploratory_sentences = [];
                // if (!assignmentTemplates.has(assignment.data.assignmentTemplate) || (!!assignment.data.assignmentTemplate && assignmentTemplates.get(assignment.data.assignmentTemplate).state !== "LOADED")) {
                if (!assignmentTemplates.has(assignment.data.assignmentTemplate)) {
                    confirmatory_summary_component = <Spinner text="Loading template... "/>;
                    exploratory_summary_component = <Spinner text="Loading template..."/>;
                } else {

                    const assignmentTemplate = assignmentTemplates.get(assignment.data.assignmentTemplate);

                    if (assignmentTemplate.state === "LOADED") {
                        fullText = assignmentTemplate.data.sentences;

                        confirmatory_sentences = latestIteration.confirmatory_summary
                            .map(id => assignmentTemplate.data.sentences.find(s => s.sentenceSubsetIndex === id))
                            .sort((a, b) => a.sentenceSubsetIndex - b.sentenceSubsetIndex)
                            .map(s => ({
                                muted: assignment.data.iterations
                                    .slice(0, assignment.data.iterations.length - 1)
                                    .some(it => it.summary.indexOf(s.untokenized_form) > -1),
                                ...s
                            }));

                        exploratory_sentences = latestIteration.exploratory_summary
                            .map(id => assignmentTemplate.data.sentences.find(s => s.sentenceSubsetIndex === id))
                            .sort((a, b) => a.sentenceSubsetIndex - b.sentenceSubsetIndex)
                            .map(s => ({
                                muted: assignment.data.iterations
                                    .slice(0, assignment.data.iterations.length - 1)
                                    .some(it => it.summary.indexOf(s.untokenized_form) > -1),
                                ...s
                            }));


                    } else {
                        confirmatory_sentences = [];
                        exploratory_sentences = [];
                    }
                    let confirmatory_annotations = this.state.annotations
                        .asArray()
                        .filter(a => confirmatory_sentences.some(cs => cs.sentenceSubsetIndex === a[0]))
                        .map(e => e[1])
                        .reduce((a, b) => a.concat(b), []);
                    let exploratory_annotations = this.state.annotations
                        .asArray()
                        .filter(a => exploratory_sentences.some(cs => cs.sentenceSubsetIndex === a[0]))
                        .map(e => e[1])
                        .reduce((a, b) => a.concat(b), []);

                    // To show exploratory summary till 2 iterations
                    if ( latestIteration.iteration <= 2 )
                    {
                        confirmatory_sentences = exploratory_sentences
                        confirmatory_annotations = exploratory_annotations
                    }

                    confirmatory_summary_component = <Summary content={confirmatory_sentences}
                                 annotations={confirmatory_annotations}
                                 callback={this.pullStateFromSummary.bind(this)}
                                 footer={"iteration #" + latestIteration.iteration + ""}/>;

                    exploratory_summary_component = <Summary content={exploratory_sentences}
                                                             annotations={exploratory_annotations}
                                                             callback={this.pullStateFromSummary.bind(this)}
                                                             footer={"iteration #" + latestIteration.iteration + ""}/>

                }
        }

        let feedback_component = <Well>No feedback available</Well>;
        switch (summary.interactions.state) {
            case "LOADING":
                feedback_component = <Spinner text="Loading recommendations..."/>;
                break;
            case "LOADED":
                // const recommendations = interactions.weights.map(e => e);
                // merge summary.interactions with existing state.interactions before pushing to the component.
                const recommendations = summary.interactions.data
                    .map(e => ({
                        item: e,
                        method: "ACCEPT_REJECT",
                        weight: e.weight,
                        priority: e.uncertainity
                    }))
                    .sort((a, b) => (b.priority - a.priority))
                    .map(e => {
                        if (this.state.interactions.has(e.item.concept)) {
                            return {
                                ...e,
                                item: {
                                    ...e.item,
                                    value: this.state.interactions.get(e.item.concept).value
                                }
                            };
                        }
                        return e;
                    });

                feedback_component =
                    <FeedbackComponent items={recommendations} callback={this.pullStateFromFeedbackComponent.bind(this)}
                                       type="ACCEPT_REJECT"/>
        }

        const personalized_summary_tooltip = <OverlayTrigger placement="top" id="confirm-tooltip-overlay"
                                                             overlay={<Tooltip id="confirm-tooltip-overlay-tooltip">In
                                                                 confirm mode, you can select regions
                                                                 in the text
                                                                 and , that you expect to improve the next
                                                                 summary.</Tooltip>}><FontAwesome
            name="question-circle"/></OverlayTrigger>;
        const explore_tooltip = <OverlayTrigger placement="top"
                                                id="explore-tooltip-overlay"
                                                overlay={<Tooltip id="explore-tooltip-overlay-tooltip">In explore mode,
                                                    you are presented sentences which
                                                    help you gather relevant concepts.
                                                    Use the mouse (or keyboard) to select regions in the summary and add
                                                    annotations, that you expect to improve the
                                                    next summary.</Tooltip>}><FontAwesome
            name="question-circle"/></OverlayTrigger>;

        let newSubmitControlPanel = <SubmitControlPanel interactions={this.state.interactions}
                                                        weights={latestIteration.weights}
                                                        callback={this.changeInteraction}/>;

        return (<Grid>
                <Row>
                    <Col xs={12}>
                        <p className="lead">
                        <small><b>Topic: </b><em>{title}</em></small><br/>
                        <Well>
                                <b>Query: </b>{narrative}
                        </Well>
                        </p>
                    </Col>
                </Row>
                <Row>
                    <Col md={8}>
                        <p>Please read the summary and give feedback highlighting important and unimportant concepts. You can add highlights by selecting parts of the text and choosing the corresponding category in the popup.
                        </p>

                        <Tabs defaultActiveKey={1} id="uncontrolled-tab-example" animation={false}>
                            <Tab eventKey={1} title={<div>Summary #{latestIteration.iteration}</div>}>
                                {confirmatory_summary_component}
                            </Tab>
                            <Tab eventKey={2} title={<div></div>} disabled>
                                {exploratory_summary_component}
                            </Tab>
                            <Tab eventKey={4} title="" disabled>
                                {feedback_component}
                            </Tab>
                            <Tab eventKey={3} title="" disabled>
                                <TopicCollectionComponent content={fullText}/>
                            </Tab>
                            <Tab eventKey={5} title="" disabled>
                                <Json json={latestIteration.weights}/>
                            </Tab>
                        </Tabs>
                        <h4>Legend:</h4>
                        <Table>
                            <thead>
                            <tr>
                                <th>Item</th>
                                <th>Meaning</th>
                            </tr>
                            </thead>
                            <tbody>
                            <tr>
                                <td>
                                    <p>
                                        <mark className="accept">Lorem ipsum</mark>
                                        dolor amet
                                    </p>
                                </td>
                                <td>Text marked as <em>good/valuable</em> information</td>
                            </tr>
                            <tr>
                                <td>
                                    <p>Lorem ipsum
                                        <mark className="reject">dolor amet</mark>
                                    </p>
                                </td>
                                <td>Text marked as <em>bad/irrelevant</em> information</td>
                            </tr>
                            <tr>
                                <td><p>Normal text</p></td>
                                <td>Sentence appears for the first time.
                                </td>
                            </tr>
                            <tr>
                                <td><p className="text-muted">Light text</p></td>
                                <td>Sentence appeared in a previous summary.
                                </td>
                            </tr>
                            </tbody>
                        </Table>


                    </Col>
                    <Col md={4}>
                        <Well>

                        <p>
                            <ControlLabel>Approximation Size (K) </ControlLabel><span>&nbsp;&nbsp;</span>
                            <FormGroup controlId="formControlsSelect">
                              <FormControl
                                onChange={this.onPickK.bind(this)}
                                inputRef={el => this.inputEl = el}
                                componentClass="select" placeholder="select">
                                <option value={0.1}>10%</option>
                                <option value={0.2}>20%</option>
                                <option value={0.3}>30%</option>
                                <option value={0.4}>40%</option>
                                <option value={0.5}>50%</option>
                                <option value={0.6}>60%</option>
                                <option value={0.7}>70%</option>
                                <option value={0.8}>80%</option>
                                <option value={0.9}>90%</option>
                                <option value={1.0}>100%</option>
                              </FormControl>
                            </FormGroup>
                        </p>
                        </Well>
                        <Well>
                            <p>
                                <Button block onClick={this.__save_feedback__.bind(this)}
                                        disabled={this.state.interactions.size < 1}>Submit</Button>
                            </p>
                            {newSubmitControlPanel}
                        </Well>
                    </Col>
                </Row>
            </Grid>
        );
    }
}


const getAssignment = (assignments, id = null) => {
    if (assignments.state === "LOADED") {
        let a;
        if (id === null) {
            a = assignments.data.filter(e => e.active === true).shift()
        } else {
            a = assignments.data.filter(e => e.id === id).shift();
        }
        return {
            state: "LOADED",
            data: a
        };
    } else {
        return {
            state: "NEW",
            data: {
                title: "",
                iterations: [{
                    summary: [],
                    iteration: -1,
                    weights: [],
                    interactions: []
                }]
            }
        }
    }
};

const getSummary = (a) => {
    const state = a.state;

    const latestIteration = a.data.iterations.reduce((x, y) => (y.iteration > x.iteration) ? y : x);

    return {
        summary: {
            state,
            data: latestIteration.summary
        },
        weights: {
            state,
            data: latestIteration.weights
        },
        interactions: {
            state,
            data: latestIteration.interactions
        }
    };
};

/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = ({assignments, assignmentTemplates, ...props}, otherProps) => {
    const assignment = getAssignment(assignments, parseInt(otherProps.routeParams.assignmentId));
    const summary = getSummary(assignment);
    return {
        store: {
            assignment,
            summary,
            assignmentTemplates
        }
    }
};

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */
const mapDispatchToProps = (dispatch) => ({
    actions: bindActionCreators(Object.assign({}, DatasetActions, SummaryActions, AssignmentActions), dispatch)
});

export default connect(mapStateToProps, mapDispatchToProps)(AnalysisContainer);

import React from 'react';
import {
    Accordion, Button, Col, Grid, Image, Media, PageHeader, Panel, ResponsiveEmbed, Row,
    Well
} from 'react-bootstrap';
import video1 from './videos/aussuchen-und-starten-eines-topics.mp4';
import thumb1 from './thumbs/aussuchen-und-starten-eines-topics.png';
import overview1 from './images/overview-analysis-container-main-parts.jpg';
import annotation2 from './images/annotations_02.jpg';
import annotationAreaTabs from './images/annotations-overview_02.jpg';
//import Video from "../../components/VideoJsComponent";
import Video from "../../components/VideoComponent";
import Lightbox from 'react-image-lightbox';
import EnlargableImage from "../../components/EnlargableImage/index";
import routeConfig from '../../config/routes';
import {LinkContainer} from "react-router-bootstrap";
import FontAwesome from 'react-fontawesome';
import Summary from "../../components/Summary/index";
import Annotation from "../../models/Annotation";

export default class HelpPage extends React.Component {

    componentDidMount() {
        window.scrollTo(0, 0);
    }

    render() {
        const summaryContent = [{
            "muted": false,
            "id": 3324,
            "concepts": ["hollywood foreign press associ", "golden globe award", "sunday night", "winner"],
            "doc_id": 2,
            "length": 14,
            "phrases": ["hollywood foreign press associ", "golden globe award", "sunday night", "winner"],
            "sent_id": 1,
            "tokens": ["Winners", "for", "the", "Hollywood", "Foreign", "Press", "Association", "'s", "Golden", "Globe", "Awards", ",", "presented", "Sunday", "night", ":", "."],
            "untokenized_concepts": ["Hollywood Foreign Press Association", "Golden Globe Awards", "Sunday night", "Winners"],
            "untokenized_form": "Winners for the Hollywood Foreign Press Association's Golden Globe Awards, presented Sunday night :.",
            "untokenized_phrases": [],
            "sentenceSubsetIndex": 47
        }, {
            "muted": true,
            "id": 3335,
            "concepts": ["privat ryan", "littl voic", "shakespear", "wake ned", "ensembl", "beauti", "movi", "life", "love"],
            "doc_id": 3,
            "length": 19,
            "phrases": ["privat ryan", "littl voic", "shakespear", "wake ned", "ensembl", "beauti", "movi", "life", "love"],
            "sent_id": 18,
            "tokens": ["ENSEMBLE", "IN", "A", "MOVIE", ":", "``", "Life", "is", "Beautiful", "''", ";", "``", "Little", "Voice", "''", ";", "``", "Saving", "Private", "Ryan", "''", ";", "``", "Shakespeare", "in", "Love", "''", ";", "``", "Waking", "Ned", "Devine", ".", "''", "."],
            "untokenized_concepts": ["Private Ryan", "Little Voice", "Shakespeare", "Waking Ned", "ENSEMBLE", "Beautiful", "MOVIE", "Life", "Love"],
            "untokenized_form": "ENSEMBLE IN A MOVIE : ``Life is Beautiful''; ``Little Voice''; ``Saving Private Ryan''; ``Shakespeare in Love''; ``Waking Ned Devine.''.",
            "untokenized_phrases": [],
            "sentenceSubsetIndex": 58
        }, {
            "muted": false,
            "id": 3760,
            "concepts": ["pedro almodovar", "foreign film", "academi", "rule"],
            "doc_id": 22,
            "length": 11,
            "phrases": ["pedro almodovar", "foreign film", "academi", "rule"],
            "sent_id": 122,
            "tokens": ["Pedro", "Almodovar", ",", "on", "the", "Academy", "'s", "rules", "on", "foreign", "films", "111QL", ";", "111QL", ";", "."],
            "untokenized_concepts": ["Pedro Almodovar", "foreign films", "Academy", "rules"],
            "untokenized_form": "Pedro Almodovar, on the Academy's rules on foreign films 111QL; 111QL;.",
            "untokenized_phrases": [],
            "sentenceSubsetIndex": 483
        }];
        return <Grid>
            <Row>
                <Col md={12}>
                    <PageHeader>Help &amp; Introduction</PageHeader>
                    <p>This page should help you to get up and running.</p>
                </Col>
            </Row>
            <Row>
                <Col md={9}>
                    <p className="lead">The main page, the <em>Analysis &amp; Personalization</em> page, is divided
                        into
                        three main areas</p>
                    <Accordion>
                        <Panel header={<h3>Area 1 &mdash; The Task area</h3>} eventKey="1">
                            <p>Area 1 contains the task description. Please, read the task and try to stick to it!.</p>
                            <p>We added the task description to the page to continously remind you, that you have a
                                specific
                                information need, that you want fulfilled.</p>
                        </Panel>
                        <Panel header={<h3>Area 2 &mdash; The Annotation area</h3>} eventKey="2">
                            <p>Area 2 features the Annotation editor, where you can select text regions and mark them as
                                important or irrelevant.</p>
                            <h3>The tabs</h3>
                            <p>There are different tabs available to you</p>
                            <dl className="dl-horizontal">
                                <dt>Personalized Summary tab</dt>
                                <dd>The personalized summary is based on your
                                    previously expressed preferences. It is <strong>personalized towards taking your
                                        previous feedback into account</strong>. The more feedback you give, the more
                                    precise it should get. <strong>Reject as much feedback here as possible, so you make
                                        sure that the personalization focuses on just whats important.</strong></dd>
                                <dt>Exploratory Summary tab</dt>
                                <dd>The exploratory summary is kind-of the opposite of the personalized summary. While
                                    the
                                    personalized summaries are based on your feedback, the exploratory summary presents
                                    excerpts to you which the system thinks are relevant, but have not received (much)
                                    feedback yet.<strong>Accept whats important here.</strong></dd>
                                <dt>Direct feedback tab</dt>
                                <dd>The direct feedback tab enables you to give &mdash; as the name implies &mdash; the
                                    opportunity to directly give feedback on the content.
                                </dd>
                                <dt>Full texts</dt>
                                <dd>Gives you direct access to all documents.</dd>
                                <dt>Weights</dt>
                                <dd>Raw access to the weights that are used.</dd>
                            </dl>
                            <EnlargableImage src={annotationAreaTabs}/>
                            <h3>Annotating text</h3>
                            <p>The annotation area is the main component for editing. To add or remove an annotation,
                                you
                                have to do:</p>
                            <ol>
                                <li>Use the mouse to select some text</li>
                                <li>Then a popup appears, where you can:
                                    <ul>
                                        <li>Mark text as relevant using <Button disabled><FontAwesome
                                            name="thumbs-o-up"/></Button>
                                        </li>
                                        <li>Mark text as irrelevant clicking <Button disabled><FontAwesome
                                            name="thumbs-o-down"/></Button></li>
                                        <li>Unmark text using <Button disabled><FontAwesome name="eraser"/></Button>
                                        </li>
                                    </ul>
                                </li>
                            </ol>
                            <Well>In many browsers, you can use double click to select a word, and triple click to
                                select a
                                whole
                                paragraph.</Well>
                            <Panel header="Try the annotator by selecting text with your cursor">
                                <Summary content={summaryContent}
                                         annotations={[new Annotation(47, 26, 53, "reject", ""),
                                             new Annotation(58, 14, 20, "accept", ""),
                                             new Annotation(58, 23, 134, "reject", ""),
                                             new Annotation(483, 13, 46, "accept", "")]}
                                    //callback={this.pullStateFromSummary.bind(this)}
                                         footer={"Simple test annotation tool."}/>
                            </Panel>
                        </Panel>
                        <Panel header={<h3>Area 3 &mdash; Selected concepts review Area</h3>} eventKey="3">
                            <p>Area 3 allows you to review the selections you will transmit to the server. The contains
                                the
                                actual information that will be used by the server to improve the summary based on your
                                feedback.</p>
                        </Panel>
                    </Accordion>
                </Col>
                <Col md={3}>
                    <EnlargableImage src={overview1}/>
                </Col>
            </Row>
            <Row>
                <Col md={6}>
                    <h1> UI in action </h1>
                    <Panel>
                        <Video src={video1} type="video/mp4" poster={thumb1} muted/>
                        <h3>Selecting a task in the File Browser </h3>
                        <p>To select a task, go to the <code> Files </code>
                            menu, and then choose some directory.If it contains a <code> Play this topic </code> Button,
                            then you can choose to process it.If you are the first to start a topic, then it may take
                            quite
                            some time until the topic has been prepared. </p>
                    </Panel>
                </Col>
            </Row>
            <Row>
                <Col>
                    <h1> Try it out </h1>
                    <LinkContainer to={routeConfig.assignments}>
                        <Panel bsStyle="primary">
                            <p className="lead text-center">
                                <FontAwesome size="4x" name="play" fixedWidth/></p> <p className="lead text-center">
                            Lets do
                            it! </p>
                        </Panel>
                    </LinkContainer>
                </Col>
            </Row>
        </Grid>
    }
};

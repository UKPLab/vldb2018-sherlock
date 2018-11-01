/**
 * Hendrik Lücke-Tieke
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 *
 * Do not use without prior consent by the copyright holder.
 *
 **/
import routeConfig from '../../config/routes';

import React from 'react';
import {connect} from 'react-redux';
import {bindActionCreators} from 'redux';
import * as DatasetActions from '../../controllers/datasets/actions';
import * as AssignmentActions from '../../controllers/assignment/actions';
import * as SummaryActions from '../../controllers/summary/actions';

import Json from 'components/JsonComponent';

import {
    Grid,
    Row,
    Col,
    Button,
    ButtonGroup,
    Panel,
    PageHeader,
    Well,
    ProgressBar
} from 'react-bootstrap';
import {LinkContainer} from "react-router-bootstrap";
import BlockingWaitComponent from '../../components/Spinner'

import FontAwesome from 'react-fontawesome';


class AssignmentPage extends React.Component {
    constructor(props) {
        super(props);
    }

    componentDidMount() {
        window.scrollTo(0, 0);
        const {location} = this.props;
        this.__loadTopic(location.query.topic);
    }

    __loadTopic(topic) {
        const {actions} = this.props;
        actions.loadTopicStatistics(topic);
    }

    _saveTaskAssignment() {
        const {actions: {createAssignment}, location: {query: {topic}}} = this.props;

        createAssignment(topic);
    }

    render() {
        const {store: {topic, users, assignments}, location} = this.props;

        if (!topic.item.documentsStatistics) {
            return <BlockingWaitComponent text="Loading document statistics..."/>
        }

        let buttonArea = (<code>Refresh the page</code>);
        switch (assignments.state) {
            case "NEW":
            case "LOADED":
                const idx = assignments.data.findIndex(e => e.topic === topic.path);
                if (idx >= 0) {
                    const item = assignments.data[idx];
                    buttonArea = (<LinkContainer to={{pathname: `${routeConfig.feedback}/${item.id}`}}>
                        <Button bsStyle="warning"> Start Task
                        </Button>
                    </LinkContainer>);
                } else {
                    // create new assignment on button click...
                    buttonArea = (
                        <Button bsStyle="primary" onClick={this._saveTaskAssignment.bind(this)}>
                            Setup Task
                        </Button>);
                }
                break;
            case "LOADING":
                buttonArea = (
                    <Button bsStyle="default" disabled>Please wait...
                        <ProgressBar active now={100}/>
                        ... we summarize the text for the first time...
                    </Button>);
                break;
            case "FAILED":
        }

        const {item} = topic;
        return (<Grid>
            {/*<Row>
                <Col>
                    <h1><FontAwesome name="flask"/>&nbsp;Task assignment</h1>
                    <p><code>{item.task.id}</code></p>
                </Col>
            </Row>*/}
            <Row>
                <Col>
                    <p className="lead">
                    <PageHeader><small><b>Topic:</b> {item.task.title} </small></PageHeader>
                    <Well><b>Query:</b> {item.task.narrative}</Well>
                    </p>
                </Col>
            </Row>
            <Row>
                <Col>
                    {/*<h2>About the documents</h2>
                    <p>It contains
                        <mark> {item.documentsStatistics.count} documents with a total
                            of {item.documentsStatistics.sum} sentences
                        </mark>
                        , with an average of {item.documentsStatistics.average}.
                    </p>*/}
                    {/*<h2>About the reference summaries</h2>*/}
                    {/*<p>There are {item.modelStatistics.count} summaries, with the*/}
                    {/*<mark> shortest consisting of {item.modelStatistics.min} sentences</mark>*/}
                    {/*, the*/}
                    {/*<mark> longest {item.modelStatistics.max} sentences</mark>*/}
                    {/*, and an average*/}
                    {/*of {item.modelStatistics.average}.*/}
                    {/*</p>*/}
                    {/*<p>A naive computer program which does not understand the task
                        needs roughly 350 interactions.
                    </p>*/}
                    <div>
                        {buttonArea}
                    </div>
                    <br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/>
                </Col>
                {/*<Col md={3}>
                    <Panel header={<FontAwesome name="info-circle" />}>
                        <h1>Goal</h1>
                        <p>Achieve good summaries with in few iterations and little feedback. Therefore, please <b>Choose wisely</b>.</p>
                        <h2>Procedure</h2>
                        <ol>
                            <li>First, you will see some statistics about your task, so you get a better feel for it.
                            </li>
                            <li>Second, you will see your task.</li>
                            <li>Third, you start the challenge, and are repeatedly asked to give feedback to what
                                extent the concepts presented to you will help fulfilling your information need.
                            </li>
                        </ol>

                    </Panel>
                </Col>*/}
            </Row>
        </Grid>);
    }
}

/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = (state) => {
    const {datasets: {topic}, assignments} = state;

    return {
        store: {
            topic,
            assignments
        }
    };

};

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */
const mapDispatchToProps = (dispatch) => ({
    actions: bindActionCreators(Object.assign({}, DatasetActions, AssignmentActions, SummaryActions), dispatch)
});

export default connect(mapStateToProps, mapDispatchToProps)(AssignmentPage);

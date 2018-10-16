/**
 * Created by hatieke on 2017-07-01.
 */

import React from 'react';
import {connect} from 'react-redux';
import {bindActionCreators} from 'redux';
import AssignmentPanel from '../../components/AssignmentPanel';
import AssignmentTemplateComponent from '../../components/AssignmentTemplateComponent';
import * as DatasetActions from '../../controllers/datasets/actions';
import * as AssignmentActions from '../../controllers/assignment/actions';
import * as AssignmentsActions from '../../controllers/assignments/actions';

import * as SummaryActions from '../../controllers/summary/actions';
import Json from 'react-json-pretty';
import {Link, browserHistory} from 'react-router'
import routeConfig from '../../config/routes';
import FontAwesome from 'react-fontawesome';
import {
    Grid,
    Row,
    Col,
    Button,
    ButtonGroup,
    Label,
    Jumbotron,
    Well, PageHeader, Panel
} from 'react-bootstrap';

class UserAssignmentContainer extends React.Component {

    static defaultProps = {
        save_callback: () => {
        }
    };

    constructor(props) {
        super(props);
    }

    componentDidMount() {
        window.scrollTo(0, 0);
    }

    _loadAssignment(assignmentId = 0) {
        const {actions, location} = this.props;
        const props = this.props;
        console.log(arguments);
        // actions.activateAssignment(assignmentId);
        // forward to "DO" page is done by the action itself on success
        // actions.getActiveAssignment();
        props.history.push(`${routeConfig.feedback}/${assignmentId}`);
    }

    render() {
        const {store} = this.props;
        console.log("Rendering UserAssignmentContainer");


        const t1 = [...store.assignmentTemplates.values()];
        const t2 = t1.filter(t => {
            // console.log(t);
            // console.log(store.assignments.data.some((a => a.topic === t.data.topic)));
            let pred = store.assignments.data.some((a => a.topic === t.data.topic));
            return !pred;
        });
        const recommendedTemplates = t2.map(e => <Col key={e.data.id} md={3} xs={6}>
            <AssignmentTemplateComponent activateAssignment={this.props.actions.createAssignment.bind(this)}
                                         template={e}/></Col>);


        let currentAssignments = store.assignments.data.map((e) => (
            <AssignmentPanel key={e.id} assignment={e} callback={this._loadAssignment.bind(this)}/>));

        return (<Grid>
            <Row>
                <Col>
                    <PageHeader>Not enough to do?
                        <small>Why dont you choose one of the following assignments:</small>
                    </PageHeader>
                    <Grid>
                        <Row>
                            {recommendedTemplates}
                        </Row>
                        {/*<Row>*/}

                            {/*<Col md={4} xs={6}>*/}
                                {/*<Panel>*/}
                                    {/*<p className="text-center lead">*/}
                                        {/*<FontAwesome size="2x" name="book" fixedWidth/></p>*/}
                                    {/*<p>Extracting the gist from a bunch of documents is hard work. Easily, some hours*/}
                                        {/*are spent on it. Computers support only at a very basic level. </p>*/}
                                {/*</Panel>*/}
                            {/*</Col>*/}
                            {/*<Col md={4} xs={6}>*/}
                                {/*<Panel><p className="lead text-center">*/}
                                    {/*<FontAwesome size="2x" name="magic" fixedWidth/></p>*/}
                                    {/*<p>Using summarization, theoretically, this can be sped up. It has already been*/}
                                        {/*shown,*/}
                                        {/*that a computer system can assist the user. However, the user will need to give*/}
                                        {/*feedback more than 300 times.</p>*/}
                                {/*</Panel>*/}
                            {/*</Col>*/}
                            {/*<Col md={4} xs={6}>*/}
                                {/*<Panel><p className="lead text-center">*/}
                                    {/*<FontAwesome size="2x" name="hourglass-2" fixedWidth/></p>*/}
                                    {/*<p>In this web application, we try new methods to bring down the 300 to something*/}
                                        {/*more*/}
                                        {/*acceptable.*/}
                                    {/*</p>*/}
                                {/*</Panel>*/}
                            {/*</Col>*/}
                        {/*</Row>*/}
                    </Grid>
                </Col>
            </Row>
            <hr/>
            <Row>
                <Col>
                    <PageHeader>Not finished yet?
                        <small>Why dont you choose one of the following assignments:</small>
                    </PageHeader>
                    {currentAssignments}
                </Col>
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
    const {assignments, users, assignmentTemplates} = state;

    return {
        store: {
            users,
            assignments,
            assignmentTemplates
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
    actions: bindActionCreators(Object.assign({}, DatasetActions, AssignmentsActions, AssignmentActions, SummaryActions), dispatch)
});

export default connect(mapStateToProps, mapDispatchToProps)(UserAssignmentContainer);

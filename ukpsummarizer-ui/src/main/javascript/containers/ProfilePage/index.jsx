/**
 * Created by hatieke on 2017-02-06.
 */


import React from 'react';
import {connect} from 'react-redux';
import {bindActionCreators} from 'redux';
import * as UserActions from '../../controllers/user/actions';
import routeConfig from '../../config/routes';

import {
    Grid,
    Row,
    Col,
    Button,
    ButtonGroup,
    Jumbotron,
    Well
} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import Summary from 'components/Summary';
import FeedbackComponent from 'components/FeedbackComponent';


export class ProfilePage extends React.Component{

    constructor(props) {
        super(props);
    };

    componentDidMount(){
        window.scrollTo(0, 0);
    }

    render() {
        const {store: {user}, actions} = props;
        // if (!user || user === null) {
        //     actions.createUser();
        // }


        return (<Grid><Row>
            <Col>
                <h1>Hi,</h1>
                <p>Thank you for using this service. In case, you participated via Amazon Mechanical Turk or some other
                    Service that pays you for using this site, you may use the following id to <code>{user.id}</code> to
                    claim your reward.</p>
            </Col>
        </Row>
        </Grid>);
    };
}

/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = (state) => ({
    store: state.users
});

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */
const mapDispatchToProps = (dispatch) => ({
    actions: bindActionCreators(Object.assign({}, UserActions), dispatch)
});

export default connect(mapStateToProps, mapDispatchToProps)(ProfilePage);

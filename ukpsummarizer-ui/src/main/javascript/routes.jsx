import React from 'react';
import {IndexRoute, Route} from 'react-router';
import { routerMiddleware, push } from 'react-router-redux';

import App from './containers/App';
import HomePage from './containers/HomePage';
import AboutPage from './containers/AboutPage';
import NotFoundPage from './containers/NotFoundPage';
import DataPage from './containers/FileBrowser';
import AnalysisContainer from './containers/AnalysisContainer';
import AssignmentPage from './containers/AssignmentPage';
import ProfilePage from './containers/ProfilePage';
import UserAssignmentsContainer from './containers/UserAssignmentsContainer';
import routeConfig from './config/routes';

import {createNewUser, verifyUserId} from './controllers/user/actions';
import {loadAssignments} from './controllers/assignments/actions';
import HelpPage from "./containers/HelpPage";
import {loadAssignmentTemplate, loadAssignmentTemplates} from "./controllers/assignmenttemplates/actions";
import {activateAssignment} from "./controllers/assignment/actions";

export default (store) => {
    const getSessionId = (nextState, replace, next) => {
        const {users: {loaded}} = store.getState();

        // check if we are logged in
        function checkLogin() {
            const {users: {user}} = store.getState();

            if (!user) {
                store.dispatch(createNewUser());
                // window.location = routeConfig.profile;
                return;
            } else {
                store.dispatch(verifyUserId());
            }
            next();
        }

        // since on start of the application we still have no auth data,
        // we first have to get it from the server (that can take time)
        if (!loaded) {

            store.dispatch(createNewUser());

            // in the mean time, we do not continue with the routing
            // and wait for the store to update our auth state
            let unsubscribe = store.subscribe(() => {
                const {users: {loaded}} = store.getState();

                // done loading, now we can check if we are logged in
                if (loaded) {
                    checkLogin();
                    unsubscribe();
                }
            });

            return;
        }

        checkLogin();
    };
    const loadUserAssignmentsContainer = () => {
        store.dispatch(loadAssignmentTemplates());
        store.dispatch(loadAssignments());
    };

    const loadAssignment = (nextState, replace, next) => {
        const {location, params, routes} = nextState;

        console.log(nextState.params.assignmentId);
        if (!params.assignmentId || Number.isNaN(parseInt(params.assignmentId))) {
            store.dispatch(push(routeConfig.assignments))
            // history.push(routeConfig.assignments);
        } else {
            store.dispatch(loadAssignmentTemplates());
            store.dispatch(activateAssignment(params.assignmentId));
            next();
        }

        // next();

// Dispatch from anywhere like normal.

    };

    return (<Route path={routeConfig.root} component={App} onEnter={getSessionId}>
                <IndexRoute component={HomePage}/>
                <Route path={routeConfig.data} component={DataPage}/>
                <Route path={routeConfig.assignments} component={UserAssignmentsContainer}
                       onEnter={loadUserAssignmentsContainer}/>
                <Route path={routeConfig.about} component={AboutPage}/>
                <Route path={routeConfig.assignment} component={AssignmentPage} onEnter={loadAssignments}/>
                <Route path={routeConfig.feedback + "/:assignmentId"} component={AnalysisContainer} onEnter={loadAssignment}/>
                <Route path={routeConfig.profile} component={ProfilePage}/>
                <Route path={routeConfig.help} component={HelpPage}/>
                <Route path="*" component={NotFoundPage}/>
            </Route>);
}

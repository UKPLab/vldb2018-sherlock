import * as Actions from './constants';

import * as UserAssignmentsActions from '../../controllers/assignments/actions';
import Rest from 'rest';
import template  from 'rest/interceptor/template';
import timeout from 'rest/interceptor/timeout';
import errorCode from 'rest/interceptor/errorCode';
import mime from 'rest/interceptor/mime';

export function createNewUser() {
    return (dispatch) => {
        console.log("trying to retrieve a createNewUser");
        dispatch({
            type: Actions.GET_USER_REQUEST
        });
        let rest = Rest.getDefaultClient();
        rest({
            method: "POST",
            path: `/users`,
            entity: "{}",
            headers: {
                "Content-Type": "application/json",
                "Accept": "application/hal+json"
            }

        }).then((success) => {
            const value = JSON.parse(success.entity);

            dispatch({
                type: Actions.GET_USER_SUCCESS,
                payload: value
            });

        }, (fail) => {
            console.log(fail);
            dispatch({
                type: Actions.GET_USER_FAIL,
                payload: fail
            });
        });
    }
}

export function verifyUserId() {
    return (dispatch, getState) => {
        const store = getState();
        const userid = store.users.user.id;

        let rest = Rest.getDefaultClient()
            .wrap(mime)
            .wrap(errorCode, {code: 400})
            .wrap(errorCode, {code: 404})
            .wrap(errorCode, {code: 500});
        rest({
            method: "GET",
            path: `/users/${userid}`,
            headers: {
                "Content-Type": "application/json",
                "Accept": "application/hal+json"
            }
        }).then((success) => {
            const value = success.entity;
            dispatch({
                type: Actions.GET_USER_SUCCESS,
                payload: value
            });
            dispatch(UserAssignmentsActions.loadAssignments())
        }, (error) => {
            dispatch({
                type: Actions.GET_USER_FAIL,
                payload: error
            });
            dispatch(createNewUser());
        });
    }
}


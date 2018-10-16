import * as Actions from './constants';
import Rest from 'rest';
import template  from 'rest/interceptor/template';
import timeout from 'rest/interceptor/timeout';
import errorCode from 'rest/interceptor/errorCode';
import mime from 'rest/interceptor/mime';

import {loadSummary} from '../../controllers/datasets/actions';
import * as SummaryActionTypes from '../../controllers/summary/constants';

export function put(entity) {
    return (dispatch) => {
        dispatch({
            type: Actions.LOAD_USER_ASSIGNMENTS_UPDATE,
            payload: entity
        });
    }
}

export function loadAssignments() {
    return (dispatch, getState) => {
        const {users} = getState();

        dispatch({
            type: Actions.LOAD_USER_ASSIGNMENTS_REQUEST
        });

        let rest = Rest.getDefaultClient()
            .wrap(mime)
            .wrap(errorCode, {code: 400})
            .wrap(errorCode, {code: 404})
            .wrap(errorCode, {code: 500});

        rest({
            method: "GET",
            path: `/assignments/all`,
            headers: {
                "u": users.user.id
            }
        }).then((success) => {
            const value = success.entity;

            dispatch({
                type: Actions.LOAD_USER_ASSIGNMENTS_SUCCESS,
                payload: value
            });
        }, (error) => {
            dispatch({
                type: Actions.LOAD_USER_ASSIGNMENTS_FAIL,
                payload: error
            })
        })
    }
}

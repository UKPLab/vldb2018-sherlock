"use strict";

import * as Actions from './constants';
import Rest from 'rest';
import template  from 'rest/interceptor/template';
import timeout from 'rest/interceptor/timeout';
import errorCode from 'rest/interceptor/errorCode';



function fetchDatasets(path = "") {
    return (dispatch) => {
        dispatch({
            type: Actions.DATASET_LIST_REQUEST
        });
        console.log("trying to access datasets");

        Rest.wrap(template)
            .wrap(timeout, {timeout: 10e6})
            .wrap(errorCode, {code: 500})(`/dataset/?path=${path}`)
            .then((response) => {
                console.log("success while accessing datasets");
                dispatch({
                    type: Actions.DATASET_LIST_SUCCESS,
                    entity: JSON.parse(response.entity),
                    path: path
                });
            }, (error) => {
                console.log("Error while accessing datasets");
                dispatch({
                    type: Actions.DATASET_LIST_FAIL,
                    payload: error
                });
            });
    }
}

export function getDatasets(path) {
    return (dispatch) => {
        console.log("trying to access getDatasets");

        return dispatch(fetchDatasets(path));
    }
}

export function loadTopicStatistics(path) {
    return (dispatch, getState) => {
        console.log("trying to load a topic");
        dispatch({
            type: Actions.TOPIC_ITEM_REQUEST
        });

        const store = getState();

        if (path === undefined || path === "") {
            path = store.assignment.data.topic;
        }
        if(path === undefined || path === null || path === "") {
            dispatch({
                type: Actions.TOPIC_ITEM_FAIL
            });

        }

        Rest.wrap(template)
            .wrap(timeout, {timeout: 10e6})
            .wrap(errorCode)(`/dataset/meta?path=${path}`)
            .then((response) => {
                dispatch({
                    type: Actions.TOPIC_ITEM_SUCCESS,
                    entity: JSON.parse(response.entity),
                    path: path
                });
                console.log("got response");
            }, (error) => {
                console.log("Error while loading the topic");
                dispatch({
                    type: Actions.TOPIC_ITEM_FAIL,
                    payload: error
                });
            });
    }
}

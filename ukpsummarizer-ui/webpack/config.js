const webpack = require('webpack');
const path = require('path');
const ExtractTextPlugin = require('extract-text-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const morgan = require('morgan');

const packageJSON = require('../package.json');

const outputPath = path.resolve(__dirname, '../dist');
const publicPath = process.env.PUBLIC_PATH || '/';

module.exports = {
    entry: [
        path.resolve(__dirname, '../src/main/javascript/index.jsx')
    ],

    output: {
        path: outputPath,
        filename: '[name].bundle.js',
        publicPath: publicPath
    },

    resolve: {
        root: path.resolve(__dirname, '../src/main/javascript'),
        extensions: ['', '.js', '.jsx']
    },

    module: {
        preLoaders: [{
            test: /\.js$/,
            loader: 'source-map-loader'
        }],
        loaders: [
            { test: /\.jsx?$/, loaders: ['babel-loader'], exclude: /node_modules/},
            { test: /\.css(\?v=\d+\.\d+\.\d+)?$/, loader: ExtractTextPlugin.extract('style-loader', ['css-loader']) },
            { test: /\.less(\?v=\d+\.\d+\.\d+)?$/, loader: ExtractTextPlugin.extract('style-loader', ['css-loader', 'less-loader']) },
            {test: /\.(woff|woff2)(\?v=\d+\.\d+\.\d+)?$/, loader: 'url?limit=10000&mimetype=application/font-woff'},
            {test: /\.ttf(\?v=\d+\.\d+\.\d+)?$/, loader: 'url?limit=10000&mimetype=application/octet-stream'},
            {test: /\.eot(\?v=\d+\.\d+\.\d+)?$/, loader: 'file'},
            {test: /\.svg(\?v=\d+\.\d+\.\d+)?$/, loader: 'url?limit=10000&mimetype=image/svg+xml'},
            {test: /\.styl$/, loader: "style!css!stylus"},
            {test: /\.([pP][nN][gG]|[jJ][pP][eE]?[gG])(\?v=\d+\.\d+\.\d+)?$/, loader: 'file-loader'},
            {test: /\.mp4(\?v=\d+\.\d+\.\d+)?$/, loader: 'file-loader'},
            {test: /\.scss(\?v=\d+\.\d+\.\d+)?$/, loader: ExtractTextPlugin.extract('style-loader', ['css-loader', 'sass-loader'])}
            ]
    },

    plugins: [
        new ExtractTextPlugin('[name].bundle.css', {allChunks: true}),
        new HtmlWebpackPlugin({
            title: 'Sherlock',
            template: path.resolve('src/main/javascript/index.ejs')
        })
    ],

    devServer: {
        host: '0.0.0.0', // in order to send links to your stuff inside the institute network
        port: process.env.PORT || 3000,
        contentBase: outputPath,
        historyApiFallback: true,
        setup: function (app) {
            app.use(morgan("combined"));
        },
        proxy: {
            '*': {
                target: 'http://localhost:8080',
                bypass: function (req, res, proxyOptions) {
                    // keep all requests that accept HTML as an answer to our index.ejs
                    if (req.headers.accept.indexOf('html') !== -1) {
                        return '/index.html';
                    } else {
                        console.log("Forwarding request to: ", proxyOptions.target, req.url);
                    }
                }
            }
        },
        watchOptions: {
            poll: 500
        }
    }
};

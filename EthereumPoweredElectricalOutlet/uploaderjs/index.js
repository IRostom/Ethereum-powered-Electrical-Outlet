var fs = require('fs');
var request = require('request');
var args = require('yargs').argv;
var filepath = require('filepath');

if (args.binary && args.board) {
    var binaryFilePath = filepath.create(args.binary).path;
    var board = args.board;

    var host = args.host ? args.host : '192.168.1.4';

    var file = fs.readFileSync(binaryFilePath);
    var contents = Buffer.from(file).toString('base64');
    var body = {
        binary: contents,
        board: board
    };

    if (args.partitions) {
        var partitionsFilePath = filepath.create(args.partitions).path;
        var partitionsFile = fs.readFileSync(partitionsFilePath);
        var partitionsContents = Buffer.from(partitionsFile).toString('base64');
        body.partitions = partitionsContents;
    }

    request.post({
        url: 'http://' + host + ':8080/api/flash',
        body: body,
        timeout: 60000,
        json: true
    }, function (error, response, body) {
        if (!error && response.statusCode == 200) {
            // console.log(body)
        } else {
            console.error(error);
            process.exit(1);
        }
    }).on('data', function (data) {
        // decompressed data as it is received
        console.log(data.toString().trim())
    })
    // .on('response', function (response) {
    //     // unmodified http.IncomingMessage object
    //     response.on('data', function (data) {
    //         // compressed data as it is received
    //         console.log('received ' + data.length + ' bytes of compressed data')
    //     })
    // });
} else {
    console.error("Please pass a binary file path");
    process.exit(1);
}

// process.argv.forEach(function (val, index, array) {
//     console.log(index + ': ' + val);
// });

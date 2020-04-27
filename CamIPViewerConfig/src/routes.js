const express = require("express");
const router = express.Router();
const { check, oneOf, validationResult } = require("express-validator/check");
const { matchedData } = require("express-validator/filter");
const fs = require("fs");

const content = "Some content!";
settings = {};

router.get("/", (req, res) => {
	res.render("index", {
		onLoadFct: "",
	});
});

router.get("/cameras", (req, res) => {
	let rawdata = fs.readFileSync("camera.json");
	settings = JSON.parse(rawdata);

	//console.log(settings.cameras)
	//
	// Prepare data payload
	//
	var data = {};
	data.cameras = settings.cameras;
	data.currentCamID = 0;

	//console.log(data)
	res.render("cameras", {
		data: data,
		onLoadFct: "initForm()",
		errors: [],
		errorMap: {},
		csrfToken: req.csrfToken(),
	});
});

router.post(
	"/cameras",
	[
		check("cam_id")
			.isAlphanumeric()
			.withMessage("Not a valid camera Id")
			.trim(),
		//check('cam_ip').trim(),
		oneOf(
			[check("cam_ip").isIP(), check("cam_ip").isFQDN()],
			"Not a valid hostname or IP"
		),
		check("cam_port").isNumeric().withMessage("Port should be a number").trim(),
		check("cam_image").trim(),
		check("cam_video").trim(),
		check("cam_user").trim(),
		check("cam_passwd").trim(),
	],
	(req, res) => {
		const errors = validationResult(req);
		if (!errors.isEmpty()) {
			//console.log('req: ', req.body)
			return res.render("cameras", {
				data: req.body,
				settings: settings,
				onLoadFct: "",
				errors: errors.array(),
				errorMap: errors.mapped(),
				csrfToken: req.csrfToken(),
			});
		}

		const reqData = matchedData(req);
		//console.log('Sanitized: ', data)
		//
		// Update settings and write the file
		//
		settings.cameras[reqData.cam_id] = reqData;

		let jsonDta = JSON.stringify(settings);

		fs.writeFile("camera.json", jsonDta, (err) => {
			if (err) {
				console.error(err);
				return;
			}
			//file written successfully
		});

		//
		// Prepare data payload
		//
		var data = {};
		data.cameras = settings.cameras;
		data.currentCamID = reqData.cam_id;

		//req.flash('success', 'Thanks for the message! I‘ll be in touch :)')
		res.render("cameras", {
			data: data,
			onLoadFct: "initForm()",
			errors: errors.array(),
			errorMap: errors.mapped(),
			csrfToken: req.csrfToken(),
		});
		//res.redirect('/cameras')
	}
);

router.get("/getsettings", (req, res) => {

  let rawdata = fs.readFileSync("camera.json");
  res.send(rawdata)
});

router.post(
	"/contact",
	[
		check("message")
			.isLength({ min: 1 })
			.withMessage("Message is required :|")
			.trim(),
		check("email")
			.isEmail()
			.withMessage("That email doesn‘t look right")
			.trim()
			.normalizeEmail(),
	],
	(req, res) => {
		const errors = validationResult(req);
		if (!errors.isEmpty()) {
			return res.render("contact", {
				data: req.body,
				errors: errors.array(),
				errorMap: errors.mapped(),
				csrfToken: req.csrfToken(),
			});
		}

		const data = matchedData(req);
		console.log("Sanitized: ", data);
		// Homework: send sanitized data in an email or persist in a db

		let jsonDta = JSON.stringify(data);
		fs.writeFile("camera.json", jsonDta, (err) => {
			if (err) {
				console.error(err);
				return;
			}
			//file written successfully
		});

		req.flash("success", "Thanks for the message! I‘ll be in touch :)");
		res.redirect("/");
	}
);

module.exports = router;

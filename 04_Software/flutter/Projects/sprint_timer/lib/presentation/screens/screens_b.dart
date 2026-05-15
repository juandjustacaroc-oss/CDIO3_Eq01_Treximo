import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:go_router/go_router.dart';
import 'package:intl/intl.dart';
import 'package:path_provider/path_provider.dart';
import 'package:share_plus/share_plus.dart';
import '../../core/theme/app_theme.dart';
import '../../domain/entities/entities.dart';
import '../../data/datasources/mock_datasource.dart';
import '../blocs/session_bloc.dart';
import '../blocs/live_session_bloc.dart';
import '../blocs/athlete_bloc.dart' as import_athlete_bloc;
import '../widgets/components.dart';

// ─────────────────────────────────────────
// SESSION DETAIL SCREEN
// ─────────────────────────────────────────
class SessionDetailScreen extends StatefulWidget {
  final String sessionId;

  const SessionDetailScreen({
    super.key,
    required this.sessionId,
  });

  @override
  State<SessionDetailScreen> createState() => _SessionDetailScreenState();
}

class _SessionDetailScreenState extends State<SessionDetailScreen> {
  late Session session;
  late List<HrSample> hrSamples;
  late List<Session> previous;
  late String athleteName;
  bool _initialized = false;

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    if (_initialized) return;
    _initialized = true;
    final sessionBloc = context.read<SessionBloc>();
    final allSessions = sessionBloc.state is SessionsLoaded 
        ? (sessionBloc.state as SessionsLoaded).sessions 
        : <Session>[];
        
    final found = allSessions.where((s) => s.sessionId == widget.sessionId).firstOrNull;
    session = found ?? Session(
      sessionId: widget.sessionId,
      athleteId: 0,
      datetimeStart: DateTime.now(),
      distanceMeters: 100,
      timeMs: 0,
      avgSpeedMps: 0,
      avgSpeedKmh: 0,
      bpmFinish: 0,
      bpmRecovery: 0,
      distanceCalibrated: false,
      ecv: 0,
    );
    hrSamples = generateHrSamples(session);
    previous = allSessions
        .where((s) =>
            s.athleteId == session.athleteId &&
            s.distanceMeters == session.distanceMeters &&
            s.sessionId != session.sessionId)
        .toList()
      ..sort((a, b) => b.datetimeStart.compareTo(a.datetimeStart));

    athleteName = mockAthletes
        .firstWhere((a) => a.athleteId == session.athleteId,
            orElse: () => Athlete(athleteId: session.athleteId, name: '#${session.athleteId}'))
        .name;
  }

  String _trend(double current, double prev, {bool higherIsBetter = true}) {
    final diff = current - prev;
    final pct = (diff / prev.abs() * 100);
    final better = higherIsBetter ? diff > 0 : diff < 0;
    final sign = diff > 0 ? '+' : '';
    return '${better ? '↑' : '↓'} $sign${pct.toStringAsFixed(1)}% vs anterior';
  }

  @override
  Widget build(BuildContext context) {
    final hasPrev = previous.isNotEmpty;
    final distColor = session.distanceMeters == 100 ? AppColors.cyan : AppColors.violet;
    final dateStr = DateFormat('dd/MM/yyyy HH:mm').format(session.datetimeStart);

    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(athleteName,
                style: const TextStyle(fontSize: 16, fontWeight: FontWeight.w600)),
            Text('${session.distanceMeters}m · $dateStr',
                style: const TextStyle(fontSize: 12, color: AppColors.textTertiary)),
          ],
        ),
        actions: [
          if (session.isPR)
            const Padding(
              padding: EdgeInsets.only(right: 8),
              child: StatusChip(label: '★ PR', color: AppColors.green, dot: false),
            ),
          IconButton(
            icon: const Icon(Icons.share, color: AppColors.textSecondary),
            onPressed: _exportCsv,
          ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // ── Big time ──
            Center(
              child: Text(session.timeFormatted,
                  style: TextStyle(
                      fontFamily: 'JetBrainsMono',
                      fontSize: 52,
                      fontWeight: FontWeight.w700,
                      color: distColor,
                      letterSpacing: -2)),
            ),
            const SizedBox(height: 4),
            Center(
              child: Text('${session.distanceMeters}m',
                  style: const TextStyle(fontSize: 14, color: AppColors.textTertiary)),
            ),
            const SizedBox(height: 24),

            // ── Metric cards grid ──
            GridView.count(
              crossAxisCount: 2,
              shrinkWrap: true,
              physics: const NeverScrollableScrollPhysics(),
              crossAxisSpacing: 10,
              mainAxisSpacing: 10,
              childAspectRatio: 1.6,
              children: [
                MetricCard(
                  label: 'Velocidad',
                  value: '${session.avgSpeedKmh.toStringAsFixed(1)} km/h',
                  subtitle: '${session.avgSpeedMps.toStringAsFixed(2)} m/s',
                  accentColor: AppColors.cyan,
                  trend: hasPrev ? _trend(session.avgSpeedMps, previous.first.avgSpeedMps) : null,
                ),
                MetricCard(
                  label: 'BPM Final',
                  value: '${session.bpmFinish} BPM',
                  subtitle: 'Al cruzar meta',
                  accentColor: AppColors.red,
                  trend: hasPrev ? _trend(session.bpmFinish.toDouble(), previous.first.bpmFinish.toDouble(), higherIsBetter: false) : null,
                  trendColor: AppColors.green,
                ),
                MetricCard(
                  label: 'ECV',
                  value: session.ecv.toStringAsFixed(4),
                  subtitle: 'v(m/s) / HR',
                  accentColor: AppColors.violet,
                  trend: hasPrev ? _trend(session.ecv, previous.first.ecv) : null,
                ),
                MetricCard(
                  label: 'BPM Recuperación',
                  value: '${session.bpmRecovery} BPM',
                  subtitle: 'A los 60s',
                  accentColor: AppColors.amber,
                ),
              ],
            ),
            const SizedBox(height: 28),

            // ── HR Chart ──
            const _SectionTitle('Frecuencia cardíaca'),
            const SizedBox(height: 12),
            _HrChart(samples: hrSamples),
            const SizedBox(height: 28),

            // ── Compare with previous ──
            if (hasPrev) ...[
              _SectionTitle('Comparación con sesiones anteriores'),
              const SizedBox(height: 12),
              _CompareTable(current: session, previous: previous.take(3).toList()),
              const SizedBox(height: 28),
            ],

            // ── Notes ──
            if (session.notes != null && session.notes!.isNotEmpty) ...[
              _SectionTitle('Notas'),
              const SizedBox(height: 8),
              Container(
                width: double.infinity,
                padding: const EdgeInsets.all(14),
                decoration: BoxDecoration(
                  color: AppColors.surface,
                  borderRadius: BorderRadius.circular(10),
                  border: Border.all(color: AppColors.border, width: 0.5),
                ),
                child: Text(session.notes!,
                    style: const TextStyle(color: AppColors.textSecondary, fontSize: 14)),
              ),
            ],

            const SizedBox(height: 40),
          ],
        ),
      ),
    );
  }

  Future<void> _exportCsv() async {
    final csv = StringBuffer();
    csv.writeln('sessionId,athleteId,fecha,distancia,timeMs,tiempo,velocidadKmh,velocidadMps,bpmFinish,bpmRecovery,ecv,isPR');
    final s = session;
    csv.writeln(
        '${s.sessionId},${s.athleteId},${s.datetimeStart.toIso8601String()},${s.distanceMeters},${s.timeMs},${s.timeFormatted},${s.avgSpeedKmh.toStringAsFixed(2)},${s.avgSpeedMps.toStringAsFixed(3)},${s.bpmFinish},${s.bpmRecovery},${s.ecv.toStringAsFixed(4)},${s.isPR}');
    final dir = await getTemporaryDirectory();
    final file = File('${dir.path}/sesion_${s.sessionId.substring(0, 8)}.csv');
    await file.writeAsString(csv.toString());
    await Share.shareXFiles([XFile(file.path)], text: 'Sesión ${s.distanceMeters}m – $athleteName');
  }
}

class _SectionTitle extends StatelessWidget {
  final String text;
  const _SectionTitle(this.text);

  @override
  Widget build(BuildContext context) => Text(text,
      style: const TextStyle(
          fontSize: 15, fontWeight: FontWeight.w600, color: AppColors.textPrimary));
}

// ── HR Chart ───────────────────────────────
class _HrChart extends StatelessWidget {
  final List<HrSample> samples;
  const _HrChart({required this.samples});

  @override
  Widget build(BuildContext context) {
    if (samples.isEmpty) {
      return const SizedBox(
        height: 140,
        child: Center(child: Text('Sin datos de HR', style: TextStyle(color: AppColors.textTertiary))),
      );
    }

    final runSamples = samples.where((s) => s.phase == HrPhase.run).toList();
    final recoverySamples = samples.where((s) => s.phase == HrPhase.recovery).toList();
    final splitMs = runSamples.isNotEmpty ? runSamples.last.offsetMs : 0;

    List<FlSpot> toSpots(List<HrSample> list) => list
        .map((s) => FlSpot(s.offsetMs / 1000.0, s.bpm.toDouble()))
        .toList();

    final allBpm = samples.map((s) => s.bpm.toDouble()).toList();
    final minY = (allBpm.reduce((a, b) => a < b ? a : b) - 10).floorToDouble();
    final maxY = (allBpm.reduce((a, b) => a > b ? a : b) + 10).ceilToDouble();
    final maxX = samples.last.offsetMs / 1000.0;

    return Container(
      height: 160,
      padding: const EdgeInsets.fromLTRB(0, 8, 16, 0),
      child: LineChart(
        LineChartData(
          minX: 0,
          maxX: maxX,
          minY: minY,
          maxY: maxY,
          backgroundColor: AppColors.surface,
          gridData: FlGridData(
            show: true,
            drawVerticalLine: false,
            getDrawingHorizontalLine: (_) => const FlLine(color: AppColors.border, strokeWidth: 0.5),
          ),
          borderData: FlBorderData(show: false),
          titlesData: FlTitlesData(
            topTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
            rightTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
            bottomTitles: AxisTitles(sideTitles: SideTitles(
              showTitles: true,
              interval: maxX / 4,
              getTitlesWidget: (v, _) => Text('${v.round()}s',
                  style: const TextStyle(fontSize: 10, color: AppColors.textTertiary)),
            )),
            leftTitles: AxisTitles(sideTitles: SideTitles(
              showTitles: true,
              interval: 20,
              reservedSize: 36,
              getTitlesWidget: (v, _) => Text('${v.round()}',
                  style: const TextStyle(fontSize: 10, color: AppColors.textTertiary)),
            )),
          ),
          lineBarsData: [
            LineChartBarData(
              spots: toSpots(runSamples),
              color: AppColors.red,
              barWidth: 2,
              isCurved: true,
              dotData: const FlDotData(show: false),
              belowBarData: BarAreaData(
                show: true,
                color: AppColors.red.withValues(alpha: 0.08),
              ),
            ),
            if (recoverySamples.isNotEmpty)
              LineChartBarData(
                spots: toSpots(recoverySamples),
                color: AppColors.amber,
                barWidth: 2,
                isCurved: true,
                dotData: const FlDotData(show: false),
                dashArray: [4, 3],
                belowBarData: BarAreaData(show: false),
              ),
          ],
          extraLinesData: ExtraLinesData(verticalLines: [
            VerticalLine(
              x: splitMs / 1000.0,
              color: AppColors.border,
              strokeWidth: 1,
              dashArray: [4, 3],
            ),
          ]),
        ),
      ),
    );
  }
}

// ── Compare table ───────────────────────────
class _CompareTable extends StatelessWidget {
  final Session current;
  final List<Session> previous;
  const _CompareTable({required this.current, required this.previous});

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(10),
        border: Border.all(color: AppColors.border, width: 0.5),
      ),
      child: Column(
        children: [
          // Header
          const Padding(
            padding: EdgeInsets.symmetric(horizontal: 14, vertical: 10),
            child: Row(
              children: [
                Expanded(child: Text('Fecha', style: TextStyle(fontSize: 12, color: AppColors.textTertiary))),
                SizedBox(width: 60, child: Text('Tiempo', style: TextStyle(fontSize: 12, color: AppColors.textTertiary), textAlign: TextAlign.end)),
                SizedBox(width: 60, child: Text('ECV', style: TextStyle(fontSize: 12, color: AppColors.textTertiary), textAlign: TextAlign.end)),
              ],
            ),
          ),
          const Divider(height: 1, color: AppColors.border),
          // Current
          _CompareRow(session: current, isCurrent: true),
          ...previous.map((s) => _CompareRow(session: s, isCurrent: false, reference: current)),
        ],
      ),
    );
  }
}

class _CompareRow extends StatelessWidget {
  final Session session;
  final bool isCurrent;
  final Session? reference;

  const _CompareRow({required this.session, required this.isCurrent, this.reference});

  @override
  Widget build(BuildContext context) {
    final dateStr = DateFormat('dd/MM/yy').format(session.datetimeStart);
    final ecvDelta = reference != null
        ? session.ecv - reference!.ecv
        : 0.0;

    return Container(
      color: isCurrent ? AppColors.cyan.withValues(alpha: 0.06) : null,
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
      child: Row(
        children: [
          Expanded(child: Text(
            isCurrent ? 'Esta sesión' : dateStr,
            style: TextStyle(
              fontSize: 13,
              fontWeight: isCurrent ? FontWeight.w600 : FontWeight.w400,
              color: isCurrent ? AppColors.cyan : AppColors.textPrimary,
            ),
          )),
          SizedBox(width: 60, child: Text(
            session.timeFormatted,
            textAlign: TextAlign.end,
            style: TextStyle(
              fontFamily: 'JetBrainsMono',
              fontSize: 13,
              fontWeight: isCurrent ? FontWeight.w700 : FontWeight.w400,
              color: isCurrent ? AppColors.cyan : AppColors.textPrimary,
            ),
          )),
          SizedBox(width: 60, child: Text(
            isCurrent
                ? session.ecv.toStringAsFixed(3)
                : '${ecvDelta >= 0 ? '+' : ''}${ecvDelta.toStringAsFixed(3)}',
            textAlign: TextAlign.end,
            style: TextStyle(
              fontFamily: 'JetBrainsMono',
              fontSize: 13,
              color: isCurrent
                  ? AppColors.violet
                  : (ecvDelta >= 0 ? AppColors.green : AppColors.red),
            ),
          )),
        ],
      ),
    );
  }
}

// ─────────────────────────────────────────
// ATHLETE DETAIL SCREEN
// ─────────────────────────────────────────
class AthleteDetailScreen extends StatelessWidget {
  final int athleteId;

  const AthleteDetailScreen({
    super.key,
    required this.athleteId,
  });

  @override
  Widget build(BuildContext context) {
    return BlocBuilder<SessionBloc, SessionBlocState>(
      builder: (context, sessionState) {
        List<Session> allSessions = [];
        if (sessionState is SessionsLoaded) {
          allSessions = sessionState.sessions;
        }

        final athleteState = context.read<import_athlete_bloc.AthleteBloc>().state;
        Athlete athlete = Athlete(athleteId: athleteId, name: '#$athleteId');
        if (athleteState is import_athlete_bloc.AthletesLoaded) {
          final found = athleteState.athletes.where((a) => a.athleteId == athleteId).firstOrNull;
          if (found != null) athlete = found;
        }

        final sessions = allSessions
            .where((s) => s.athleteId == athleteId)
            .toList()
          ..sort((a, b) => b.datetimeStart.compareTo(a.datetimeStart));

        final pr100 = sessions
            .where((s) => s.distanceMeters == 100)
            .toList()
          ..sort((a, b) => a.timeMs.compareTo(b.timeMs));
        final pr50 = sessions
            .where((s) => s.distanceMeters == 50)
            .toList()
          ..sort((a, b) => a.timeMs.compareTo(b.timeMs));

        return Scaffold(
          backgroundColor: AppColors.bg,
          appBar: AppBar(
            title: Text(athlete.name),
            backgroundColor: AppColors.bg,
            foregroundColor: AppColors.textPrimary,
          ),
          body: SingleChildScrollView(
            padding: const EdgeInsets.all(20),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Container(
                      width: 64,
                      height: 64,
                      decoration: BoxDecoration(
                        color: AppColors.violet.withValues(alpha: 0.15),
                        shape: BoxShape.circle,
                      ),
                      child: Center(
                        child: Text('#$athleteId',
                            style: const TextStyle(
                                fontSize: 18,
                                fontWeight: FontWeight.w700,
                                color: AppColors.violet)),
                      ),
                    ),
                    const SizedBox(width: 16),
                    Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(athlete.name,
                            style: const TextStyle(
                                fontSize: 18, fontWeight: FontWeight.w700, color: AppColors.textPrimary)),
                        if (athlete.notes != null)
                          Text(athlete.notes!,
                              style: const TextStyle(fontSize: 13, color: AppColors.textTertiary)),
                        Text('${sessions.length} sesiones registradas',
                            style: const TextStyle(fontSize: 12, color: AppColors.textTertiary)),
                      ],
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                Row(
                  children: [
                    if (pr100.isNotEmpty) _PRCard(distance: '100m', session: pr100.first),
                    if (pr100.isNotEmpty && pr50.isNotEmpty) const SizedBox(width: 10),
                    if (pr50.isNotEmpty) _PRCard(distance: '50m', session: pr50.first),
                  ],
                ),
                const SizedBox(height: 24),
                const Text('Últimas 5 sesiones',
                    style: TextStyle(fontSize: 15, fontWeight: FontWeight.w600, color: AppColors.textPrimary)),
                const SizedBox(height: 12),
                for (final s in sessions.take(5))
                  SessionTile(
                    session: s,
                    athleteName: athlete.name,
                    onTap: () => context.push('/session/${s.sessionId}'),
                  ),
                if (sessions.length > 5) ...[
                  const SizedBox(height: 8),
                  Center(
                    child: TextButton(
                      onPressed: () => context.push('/history'),
                      child: const Text('Ver todo el historial',
                          style: TextStyle(color: AppColors.cyan)),
                    ),
                  ),
                ],
              ],
            ),
          ),
        );
      },
    );
  }
}

class _PRCard extends StatelessWidget {
  final String distance;
  final Session session;
  const _PRCard({required this.distance, required this.session});

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: Container(
        padding: const EdgeInsets.all(14),
        decoration: BoxDecoration(
          color: AppColors.green.withOpacity(0.08),
          borderRadius: BorderRadius.circular(10),
          border: Border.all(color: AppColors.green.withOpacity(0.3), width: 0.8),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('★ PR $distance',
                style: const TextStyle(fontSize: 11, color: AppColors.green, fontWeight: FontWeight.w600)),
            const SizedBox(height: 4),
            Text(session.timeFormatted,
                style: const TextStyle(
                    fontFamily: 'JetBrainsMono',
                    fontSize: 20,
                    fontWeight: FontWeight.w700,
                    color: AppColors.green)),
            Text(DateFormat('dd/MM/yy').format(session.datetimeStart),
                style: const TextStyle(fontSize: 11, color: AppColors.textTertiary)),
          ],
        ),
      ),
    );
  }
}

// ─────────────────────────────────────────
// NEW SESSION SCREEN
// ─────────────────────────────────────────
class NewSessionScreen extends StatelessWidget {
  const NewSessionScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('Nueva sesión'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: BlocConsumer<LiveSessionBloc, LiveState>(
        listener: (context, state) {
          if (state.sessionState == SessionState.recovery &&
              state.finishedSession != null) {
            context.read<SessionBloc>().add(const LoadSessions());
          }
        },
        builder: (context, state) {
          return SingleChildScrollView(
            padding: const EdgeInsets.all(20),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Athlete selector
                const Text('Atleta',
                    style: TextStyle(fontSize: 13, color: AppColors.textTertiary)),
                const SizedBox(height: 8),
                _AthleteSelector(selected: state.selectedAthlete),
                const SizedBox(height: 20),

                // Distance selector
                const Text('Distancia',
                    style: TextStyle(fontSize: 13, color: AppColors.textTertiary)),
                const SizedBox(height: 8),
                Row(
                  children: [
                    for (final d in [50, 100])
                      Padding(
                        padding: const EdgeInsets.only(right: 10),
                        child: GestureDetector(
                          onTap: () => context.read<LiveSessionBloc>().add(LiveSelectDistance(d)),
                          child: Container(
                            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
                            decoration: BoxDecoration(
                              color: state.selectedDistance == d
                                  ? AppColors.cyan.withValues(alpha: 0.15)
                                  : AppColors.surface,
                              borderRadius: BorderRadius.circular(10),
                              border: Border.all(
                                color: state.selectedDistance == d
                                    ? AppColors.cyan
                                    : AppColors.border,
                                width: state.selectedDistance == d ? 1.5 : 0.5,
                              ),
                            ),
                            child: Text('${d}m',
                                style: TextStyle(
                                    fontSize: 16,
                                    fontWeight: FontWeight.w700,
                                    color: state.selectedDistance == d
                                        ? AppColors.cyan
                                        : AppColors.textSecondary)),
                          ),
                        ),
                      ),
                  ],
                ),
                const SizedBox(height: 30),

                // Status display
                _SessionStatusDisplay(state: state),
                const SizedBox(height: 30),

                // Controls
                if (state.sessionState == SessionState.idle ||
                    state.sessionState == SessionState.ready) ...[
                  SizedBox(
                    width: double.infinity,
                    child: ElevatedButton.icon(
                      onPressed: state.selectedAthlete == null
                          ? null
                          : () => context.read<LiveSessionBloc>().add(LiveSimulateRace()),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: AppColors.cyan,
                        foregroundColor: AppColors.bg,
                        padding: const EdgeInsets.symmetric(vertical: 16),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                      icon: const Icon(Icons.play_arrow_rounded),
                      label: const Text('Simular carrera (mock)',
                          style: TextStyle(fontWeight: FontWeight.w700, fontSize: 15)),
                    ),
                  ),
                  const SizedBox(height: 8),
                  Center(
                    child: Text('El hardware START/FINISH iniciará automáticamente',
                        style: const TextStyle(fontSize: 12, color: AppColors.textTertiary),
                        textAlign: TextAlign.center),
                  ),
                ],

                if (state.sessionState == SessionState.recovery ||
                    state.sessionState == SessionState.finished) ...[
                  SizedBox(
                    width: double.infinity,
                    child: OutlinedButton(
                      onPressed: () => context.read<LiveSessionBloc>().add(LiveReset()),
                      style: OutlinedButton.styleFrom(
                        foregroundColor: AppColors.textSecondary,
                        side: const BorderSide(color: AppColors.border, width: 0.8),
                        padding: const EdgeInsets.symmetric(vertical: 14),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                      child: const Text('Nueva carrera'),
                    ),
                  ),
                  if (state.finishedSession != null) ...[
                    const SizedBox(height: 10),
                    SizedBox(
                      width: double.infinity,
                      child: TextButton(
                        onPressed: () => context.push('/session/${state.finishedSession!.sessionId}'),
                        child: const Text('Ver detalle de sesión →',
                            style: TextStyle(color: AppColors.cyan)),
                      ),
                    ),
                  ],
                ],
              ],
            ),
          );
        },
      ),
    );
  }
}

class _AthleteSelector extends StatelessWidget {
  final Athlete? selected;
  const _AthleteSelector({this.selected});

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: () => showModalBottomSheet(
        context: context,
        backgroundColor: AppColors.surface,
        shape: const RoundedRectangleBorder(
            borderRadius: BorderRadius.vertical(top: Radius.circular(16))),
        builder: (_) => Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              const Text('Seleccionar atleta',
                  style: TextStyle(fontWeight: FontWeight.w600, fontSize: 16, color: AppColors.textPrimary)),
              const SizedBox(height: 12),
              for (final a in mockAthletes)
                ListTile(
                  leading: CircleAvatar(
                    backgroundColor: AppColors.violet.withValues(alpha: 0.15),
                    child: Text('#${a.athleteId}',
                        style: const TextStyle(color: AppColors.violet, fontWeight: FontWeight.w700, fontSize: 13)),
                  ),
                  title: Text(a.name, style: const TextStyle(color: AppColors.textPrimary)),
                  subtitle: Text(a.notes ?? '', style: const TextStyle(color: AppColors.textTertiary)),
                  trailing: selected?.athleteId == a.athleteId
                      ? const Icon(Icons.check, color: AppColors.cyan)
                      : null,
                  onTap: () {
                    Navigator.pop(context);
                    context.read<LiveSessionBloc>().add(LiveSelectAthlete(a));
                  },
                ),
            ],
          ),
        ),
      ),
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 14),
        decoration: BoxDecoration(
          color: AppColors.surface,
          borderRadius: BorderRadius.circular(10),
          border: Border.all(color: AppColors.border, width: 0.5),
        ),
        child: Row(
          children: [
            if (selected != null)
              CircleAvatar(
                radius: 14,
                backgroundColor: AppColors.violet.withOpacity(0.15),
                child: Text('#${selected!.athleteId}',
                    style: const TextStyle(color: AppColors.violet, fontSize: 11, fontWeight: FontWeight.w700)),
              )
            else
              const Icon(Icons.person, color: AppColors.textTertiary, size: 20),
            const SizedBox(width: 12),
            Text(
              selected?.name ?? 'Seleccionar atleta...',
              style: TextStyle(
                fontSize: 15,
                color: selected != null ? AppColors.textPrimary : AppColors.textTertiary,
                fontWeight: selected != null ? FontWeight.w600 : FontWeight.w400,
              ),
            ),
            const Spacer(),
            const Icon(Icons.expand_more, color: AppColors.textTertiary),
          ],
        ),
      ),
    );
  }
}

class _SessionStatusDisplay extends StatelessWidget {
  final LiveState state;
  const _SessionStatusDisplay({required this.state});

  @override
  Widget build(BuildContext context) {
    final isRunning = state.sessionState == SessionState.inProgress;
    final isFinished = state.sessionState == SessionState.finished ||
        state.sessionState == SessionState.recovery;
    final distColor = state.selectedDistance == 100 ? AppColors.cyan : AppColors.violet;

    final ms = state.elapsedMs;
    final sec = ms ~/ 1000;
    final tenth = (ms % 1000) ~/ 100;
    final min = sec ~/ 60;
    final s = sec % 60;
    final timeStr = min > 0
        ? '${min.toString().padLeft(2, '0')}:${s.toString().padLeft(2, '0')}.$tenth'
        : '${s.toString().padLeft(2, '0')}.$tenth';

    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(16),
        border: Border.all(
          color: isRunning ? AppColors.cyan.withValues(alpha: 0.4) : AppColors.border,
          width: isRunning ? 1.5 : 0.5,
        ),
      ),
      child: Column(
        children: [
          // State chip
          StatusChip(
            label: switch (state.sessionState) {
              SessionState.ready => 'Listo',
              SessionState.inProgress => 'En carrera',
              SessionState.finished => 'Finalizado',
              SessionState.recovery => 'Recuperación',
              _ => 'En espera',
            },
            color: switch (state.sessionState) {
              SessionState.ready => AppColors.amber,
              SessionState.inProgress => AppColors.cyan,
              SessionState.finished => AppColors.green,
              SessionState.recovery => AppColors.amber,
              _ => AppColors.textTertiary,
            },
          ),
          const SizedBox(height: 16),

          // Big time
          Text(
            ms == 0 && !isRunning && !isFinished ? '00.0' : timeStr,
            style: TextStyle(
                fontFamily: 'JetBrainsMono',
                fontSize: 48,
                fontWeight: FontWeight.w700,
                color: isFinished ? AppColors.green : distColor,
                letterSpacing: -2),
          ),

          // HR
          if (state.currentHr > 0) ...[
            const SizedBox(height: 12),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Icon(Icons.favorite, color: AppColors.red, size: 16),
                const SizedBox(width: 6),
                Text('${state.currentHr} BPM',
                    style: const TextStyle(
                        fontFamily: 'JetBrainsMono',
                        fontSize: 16,
                        color: AppColors.red,
                        fontWeight: FontWeight.w600)),
              ],
            ),
          ],
        ],
      ),
    );
  }
}

// ─────────────────────────────────────────
// CONNECTION SCREEN
// ─────────────────────────────────────────
class ConnectionScreen extends StatelessWidget {
  const ConnectionScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final devices = [
      (name: 'Cerebro (cronómetro)', ok: true),
      (name: 'Peón salida', ok: true),
      (name: 'Peón llegada', ok: false),
      (name: 'Banda HR', ok: true),
    ];

    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('Estado del sistema'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            ConnectionPanel(devices: devices),
            const SizedBox(height: 16),
            Container(
              padding: const EdgeInsets.all(14),
              decoration: BoxDecoration(
                color: AppColors.surface,
                borderRadius: BorderRadius.circular(10),
                border: Border.all(color: AppColors.border, width: 0.5),
              ),
              child: const Column(
                children: [
                  _InfoRow(label: 'Última señal', value: 'Hace 2s'),
                  _InfoRow(label: 'RSSI', value: '−62 dBm'),
                  _InfoRow(label: 'Firmware', value: '1.3.2'),
                  _InfoRow(label: 'Protocolo', value: 'ESP-NOW (mock)'),
                ],
              ),
            ),
            const SizedBox(height: 20),
            Container(
              padding: const EdgeInsets.all(14),
              decoration: BoxDecoration(
                color: AppColors.amber.withValues(alpha: 0.08),
                borderRadius: BorderRadius.circular(10),
                border: Border.all(color: AppColors.amber.withValues(alpha: 0.3), width: 0.8),
              ),
              child: const Row(
                children: [
                  Icon(Icons.info_outline, color: AppColors.amber, size: 18),
                  SizedBox(width: 10),
                  Expanded(
                    child: Text(
                      'Modo mock activo. El protocolo BLE/WebSocket se conectará reemplazando MockSportDataSource por RealDataSource.',
                      style: TextStyle(fontSize: 13, color: AppColors.amber),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class _InfoRow extends StatelessWidget {
  final String label;
  final String value;
  const _InfoRow({required this.label, required this.value});

  @override
  Widget build(BuildContext context) => Padding(
    padding: const EdgeInsets.symmetric(vertical: 7),
    child: Row(
      children: [
        Text(label, style: const TextStyle(fontSize: 13, color: AppColors.textTertiary)),
        const Spacer(),
        Text(value,
            style: const TextStyle(
                fontSize: 13, color: AppColors.textPrimary, fontWeight: FontWeight.w500)),
      ],
    ),
  );
}

// ─────────────────────────────────────────
// SETTINGS SCREEN
// ─────────────────────────────────────────
class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  bool _useKmh = true;
  int _defaultDistance = 100;
  final String _ecvFormula = 'v(m/s) / HR_avg';

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      appBar: AppBar(
        title: const Text('Ajustes'),
        backgroundColor: AppColors.bg,
        foregroundColor: AppColors.textPrimary,
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _SettingsSection(
              title: 'Unidades',
              children: [
                SwitchListTile(
                  title: const Text('Velocidad en km/h', style: TextStyle(color: AppColors.textPrimary)),
                  subtitle: Text(_useKmh ? 'km/h' : 'm/s',
                      style: const TextStyle(color: AppColors.textTertiary)),
                  value: _useKmh,
                  activeThumbColor: AppColors.cyan,
                  onChanged: (v) => setState(() => _useKmh = v),
                ),
              ],
            ),
            const SizedBox(height: 16),
            _SettingsSection(
              title: 'Sesiones',
              children: [
                ListTile(
                  title: const Text('Distancia por defecto', style: TextStyle(color: AppColors.textPrimary)),
                  trailing: DropdownButton<int>(
                    value: _defaultDistance,
                    dropdownColor: AppColors.surfaceHigh,
                    items: [50, 100].map((d) => DropdownMenuItem(
                      value: d,
                      child: Text('${d}m', style: const TextStyle(color: AppColors.textPrimary)),
                    )).toList(),
                    onChanged: (v) => setState(() => _defaultDistance = v!),
                    underline: const SizedBox(),
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            _SettingsSection(
              title: 'Fórmula ECV',
              children: [
                Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Container(
                        padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
                        decoration: BoxDecoration(
                          color: AppColors.surfaceHigh,
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: Text(_ecvFormula,
                            style: const TextStyle(
                                fontFamily: 'JetBrainsMono',
                                fontSize: 14,
                                color: AppColors.violet)),
                      ),
                      const SizedBox(height: 8),
                      const Text('ECV: Eficiencia Cardiovascular. Mide la velocidad obtenida por cada latido.',
                          style: TextStyle(fontSize: 12, color: AppColors.textTertiary)),
                      const SizedBox(height: 8),
                      const Text('Para cambiar la fórmula, edita EcvCalculator en domain/usecases.',
                          style: TextStyle(fontSize: 11, color: AppColors.textTertiary)),
                    ],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            _SettingsSection(
              title: 'Exportación',
              children: [
                ListTile(
                  title: const Text('Exportar todas las sesiones',
                      style: TextStyle(color: AppColors.textPrimary)),
                  trailing: const Icon(Icons.share, color: AppColors.textTertiary, size: 18),
                  onTap: () {
                    ScaffoldMessenger.of(context).showSnackBar(const SnackBar(
                      content: Text('Exportación CSV disponible en detalle de sesión'),
                      backgroundColor: AppColors.surface,
                    ));
                  },
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _SettingsSection extends StatelessWidget {
  final String title;
  final List<Widget> children;

  const _SettingsSection({required this.title, required this.children});

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(title.toUpperCase(),
            style: const TextStyle(fontSize: 11, fontWeight: FontWeight.w600,
                letterSpacing: 0.8, color: AppColors.textTertiary)),
        const SizedBox(height: 8),
        Container(
          decoration: BoxDecoration(
            color: AppColors.surface,
            borderRadius: BorderRadius.circular(12),
            border: Border.all(color: AppColors.border, width: 0.5),
          ),
          child: Column(children: children),
        ),
      ],
    );
  }
}
